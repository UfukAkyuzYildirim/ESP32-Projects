#include <Arduino.h>
#include <IRutils.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

#include "Config.h"
#include "ButtonHandler.h"
#include "StatusLed.h"
#include "IrReader.h"
#include "IrSender.h"
#include "WifiManager.h"

// --- Global Nesneler ---
ButtonHandler button(PIN_BUTTON);
StatusLed statusLed(PIN_LED);
IrReader irReader(PIN_IR_RECV);
IrSender irSender(PIN_IR_SEND);
WifiManager wifiManager(WIFI_SSID, WIFI_PASS);
ESP8266WebServer server(HTTP_PORT);

// --- Degiskenler ---
decode_results lastResults;      // Son okunan IR kodu
uint16_t *savedRawData = nullptr;// RAW veriyi saklamak icin pointer
uint16_t savedRawLength = 0;     // RAW veri uzunlugu
bool hasNewData = false;         // API icin yeni veri var mi bayragi

bool isLearningMode = false;     // Ogrenme Modu Durumu
unsigned long learningTimer = 0; // 30 saniyelik zaman asimi sayaci

// --- Yardimci Fonksiyonlar ---

// Hafizayi temizler
void clearSavedData() {
    if (savedRawData != nullptr) {
        delete [] savedRawData;
        savedRawData = nullptr;
    }
    savedRawLength = 0;
    hasNewData = false;
    // lastResults sifirlama gerekmiyor, uzerine yaziliyor
}

// Ogrenme Modunu Baslat
void startLearningMode() {
    if (isLearningMode) {
        // Zaten aciksa sadece sureyi uzat
        learningTimer = millis();
        Serial.println(">> Ogrenme Modu Suresi Uzatildi.");
        return; 
    }
    
    Serial.println(">> OGRENME MODU AKTIF! (30 sn sure basladi)");
    isLearningMode = true;
    learningTimer = millis();
    
    // IR Aliciyi baslat/temizle
    irReader.begin(); 
    irReader.resume();
}

// Ogrenme Modunu Bitir
void stopLearningMode() {
    if (!isLearningMode) return;
    
    Serial.println(">> Ogrenme Modu KAPATILDI. (Beklemeye gecildi)");
    isLearningMode = false;
    statusLed.off();
}

// --- HTTP Handler Fonksiyonlari ---

void handleRoot() {
    String msg = "ESP8266 IR Gateway Calisiyor.\n";
    msg += "IP: " + wifiManager.getIp() + "\n";
    msg += "Durum: " + String(isLearningMode ? "OGRENME MODU" : "Beklemede");
    server.send(200, "text/plain", msg);
}

// GET /ir/learn
void handleStartLearn() {
    startLearningMode();
    server.send(200, "application/json", "{\"status\":\"Learning Mode Started\",\"timeout\":30}");
}

// GET /ir/data
void handleGetData() {
    if (!hasNewData) {
        server.send(204, "application/json", ""); // No Content
        return;
    }

    // JSON Olusturma
    // RAW data buyuk olabilir, DynamicJsonDocument boyutunu iyi ayarlamak lazim.
    // Klima kumandalari icin 200-300 elemanli array olabilir. 
    // 4KB (4096) genelde yeterli.
    DynamicJsonDocument doc(4096);

    doc["protocol"] = typeToString(lastResults.decode_type);
    doc["protocol_id"] = (int)lastResults.decode_type;
    doc["value"] = (uint32_t)lastResults.value; // 64 bit destegi gerekirse string'e cevirilmeli
    doc["bits"] = lastResults.bits;
    doc["raw_len"] = savedRawLength;

    JsonArray rawArray = doc.createNestedArray("raw_data");
    if (savedRawData != nullptr) {
        for (uint16_t i = 0; i < savedRawLength; i++) {
            rawArray.add(savedRawData[i]);
        }
    }

    String jsonOutput;
    serializeJson(doc, jsonOutput);

    server.send(200, "application/json", jsonOutput);

    // Veri gonderildikten sonra temizle (Read-Once mantigi)
    Serial.println(">> Veri API uzerinden cekildi ve silindi.");
    clearSavedData();
    stopLearningMode(); // Veri alindiginda ogrenme modundan cikmak mantikli olabilir
}

// POST /ir/send
void handleSend() {
    if (!server.hasArg("plain")) {
        server.send(400, "text/plain", "Body missing");
        return;
    }

    String body = server.arg("plain");
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, body);

    if (error) {
        server.send(400, "text/plain", "Invalid JSON");
        return;
    }

    Serial.println(">> API'den Gonderme Istegi Geldi...");

    // RAW veri var mi kontrol et
    bool isRaw = doc.containsKey("raw_data");
    
    if (isRaw) {
        JsonArray rawArray = doc["raw_data"];
        uint16_t len = rawArray.size();
        uint16_t *rawBuffer = new uint16_t[len];
        
        for (uint16_t i = 0; i < len; i++) {
            rawBuffer[i] = rawArray[i];
        }
        
        // Frekans genelde 38kHz
        uint16_t freq = 38; 
        if (doc.containsKey("freq")) freq = doc["freq"];

        Serial.printf(">> RAW Gonderme Basliyor (%d eleman)...\n", len);
        
        IRsend tempSender(PIN_IR_SEND);
        tempSender.begin();
        tempSender.sendRaw(rawBuffer, len, freq);
        
        delete[] rawBuffer;
    } else {
        // Standart protokol gonderme
        // Burasi biraz karisik cunku IRremoteESP8266 send fonksiyonlari ayri ayri.
        // decode_type enum'ina gore switch-case yapmak lazim ama cok uzun.
        // Simdilik sadece RAW uzerinden gitmek en garantisi.
        // Veya "send(decode_type, value, bits)" fonksiyonu generic yok.
        
        Serial.println("!! UYARI: Su an sadece RAW gonderme tam destekleniyor.");
        // Gerekirse buraya protokol bazli switch-case eklenebilir.
        // Ancak hub tarafi zaten RAW veriyi de kaydettigi icin RAW gondermeyi tercih etmeliyiz.
    }

    server.send(200, "application/json", "{\"status\":\"Signal Sent\"}");
}

// --- Setup ---
void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n\n--- IR Gateway Sistemi Baslatildi ---");

    // Donanim Baslatma
    button.begin();
    statusLed.begin();
    irSender.begin();
    
    // Wi-Fi Baslatma
    wifiManager.begin();

    // Server Rotalari
    server.on("/", HTTP_GET, handleRoot);
    server.on("/ir/learn", HTTP_GET, handleStartLearn); // POST da olabilir ama tarayicidan test icin GET kolay
    server.on("/ir/data", HTTP_GET, handleGetData);
    server.on("/ir/send", HTTP_POST, handleSend);

    server.begin();
    Serial.println("HTTP Sunucu Baslatildi.");

    // IR Okuyucu (Baslangicta pasif olabilir ama startLearningMode icinde aciliyor)
    irReader.begin();

    Serial.println("Sistem Hazir.");
}

// --- Loop ---
void loop() {
    server.handleClient();
    wifiManager.loop();

    unsigned long currentMillis = millis();

    // 1. Buton Kontrolleri
    // ------------------------------------------------
    if (button.isLongPressed()) {
        if (!isLearningMode) {
            startLearningMode();
        }
    }

    if (button.isTripleClicked()) {
        if (isLearningMode) {
            Serial.println(">> Manuel Iptal.");
            stopLearningMode();
        }
    }


    // 2. Ogrenme Modu Mantigi
    // ------------------------------------------------
    if (isLearningMode) {
        statusLed.blink(500, 1000);

        if (currentMillis - learningTimer > LEARN_MODE_TIMEOUT) {
            Serial.println(">> Zaman asimi!");
            stopLearningMode();
        }

        if (irReader.loop()) {
            Serial.println(">> Sinyal Algilandi!");
            
            learningTimer = currentMillis;
            clearSavedData();

            lastResults = irReader.getResults();
            
            // RAW Kopyalama
            savedRawData = resultToRawArray(&lastResults); 
            savedRawLength = getCorrectedRawLength(&lastResults);
            hasNewData = true; // API icin bayrak kaldir
            
            Serial.print("Protocol: ");
            Serial.print(typeToString(lastResults.decode_type));
            Serial.println(" >> Hafizaya Alindi. API ile cekilmeyi bekliyor.");

            irReader.resume();
        }
    } else {
        statusLed.off();
    }
}
