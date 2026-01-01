#include <Arduino.h>
#include <IRutils.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <vector>

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
WifiManager wifiManager;
ESP8266WebServer server(HTTP_PORT);

// --- Veri Yapisi ---
struct CapturedSignal {
    String protocolName;
    decode_type_t protocolId;
    uint64_t value;
    uint16_t bits;
    std::vector<uint16_t> rawData; // Sadece UNKNOWN ise dolar
    bool hasRaw;
};

// Maksimum kac sinyal tutulacak
#define MAX_SIGNAL_BUFFER 10

// --- Degiskenler ---
std::vector<CapturedSignal> signalBuffer; // Sinyal listesi

bool isLearningMode = false;     // Ogrenme Modu Durumu
unsigned long learningTimer = 0; // 30 saniyelik zaman asimi sayaci

// --- Yardimci Fonksiyonlar ---

// Hafizayi temizler
void clearSignalBuffer() {
    signalBuffer.clear();
}

// Yeni sinyali listeye ekle (Tekrarlari onler)
void addSignalToBuffer(decode_results *results) {
    if (signalBuffer.size() >= MAX_SIGNAL_BUFFER) {
        Serial.println(">> Buffer dolu! Yeni sinyal kabul edilmiyor.");
        return;
    }

    for (const auto& s : signalBuffer) {
        if (s.protocolId == results->decode_type && s.value == results->value) {
            Serial.println(">> Bu sinyal zaten listede var. Eklenmedi.");
            return;
        }
    }

    CapturedSignal newSignal;
    newSignal.protocolName = typeToString(results->decode_type);
    newSignal.protocolId = results->decode_type;
    newSignal.value = results->value;
    newSignal.bits = results->bits;
    
    if (results->decode_type == UNKNOWN) {
        newSignal.hasRaw = true;
        uint16_t *rawArray = resultToRawArray(results);
        uint16_t rawLen = getCorrectedRawLength(results);
        
        newSignal.rawData.assign(rawArray, rawArray + rawLen);
        
        delete [] rawArray; 
    } else {
        newSignal.hasRaw = false;
    }

    signalBuffer.push_back(newSignal);
    Serial.printf(">> Sinyal Eklendi. Buffer: %d/%d\n", signalBuffer.size(), MAX_SIGNAL_BUFFER);
}


// Ogrenme Modunu Baslat
void startLearningMode() {
    if (isLearningMode) {
        learningTimer = millis();
        Serial.println(">> Ogrenme Modu Suresi Uzatildi.");
        return; 
    }
    
    Serial.println(">> OGRENME MODU AKTIF! (30 sn sure basladi)");
    isLearningMode = true;
    learningTimer = millis();
    clearSignalBuffer(); 
    
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
    msg += "WiFi: " + String(wifiManager.isConnected() ? "STA" : (wifiManager.isSetupApActive() ? "AP(SETUP)" : "OFF")) + "\n";
    if (wifiManager.isSetupApActive()) {
        msg += "SetupAP SSID: " + wifiManager.getSetupApSsid() + "\n";
        msg += "WiFi ayarlamak icin: POST /wifi/config (JSON)\n";
    }
    msg += "Durum: " + String(isLearningMode ? "OGRENME MODU" : "Beklemede") + "\n";
    msg += "Yakalanan Sinyal: " + String(signalBuffer.size());
    server.send(200, "text/plain", msg);
}

// GET /status
void handleStatus() {
    JsonDocument doc;
    doc["ip"] = wifiManager.getIp();
    doc["wifi_mode"] = wifiManager.isConnected() ? "STA" : (wifiManager.isSetupApActive() ? "AP" : "OFF");
    doc["setup_ap_ssid"] = wifiManager.getSetupApSsid();
    doc["learning"] = isLearningMode;
    doc["buffer_count"] = (int)signalBuffer.size();

    String out;
    serializeJson(doc, out);
    server.send(200, "application/json", out);
}

// POST /wifi/config
// Body: {"ssid":"...","pass":"..."}
void handleWifiConfig() {
    if (!wifiManager.isSetupApActive() && wifiManager.isConnected()) {
        server.send(403, "text/plain", "WiFi config only allowed in setup mode");
        return;
    }

    if (!server.hasArg("plain")) {
        server.send(400, "text/plain", "Body missing");
        return;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));
    if (error) {
        server.send(400, "text/plain", "Invalid JSON");
        return;
    }

    if (!doc["ssid"].is<const char*>()) {
        server.send(400, "text/plain", "Missing ssid");
        return;
    }

    String ssid = String((const char*)doc["ssid"]);
    String pass = doc["pass"].is<const char*>() ? String((const char*)doc["pass"]) : String("");

    if (ssid.length() == 0 || ssid.length() > 32) {
        server.send(400, "text/plain", "Invalid ssid length");
        return;
    }
    if (pass.length() > 64) {
        server.send(400, "text/plain", "Invalid pass length");
        return;
    }

    bool ok = wifiManager.saveCredentials(ssid, pass);
    if (!ok) {
        server.send(500, "text/plain", "Failed to save credentials");
        return;
    }

    server.send(200, "application/json", "{\"status\":\"saved\",\"restarting\":true}");
    delay(250);
    ESP.restart();
}

// GET /ir/learn
void handleStartLearn() {
    startLearningMode();
    server.send(200, "application/json", "{\"status\":\"Learning Mode Started\",\"timeout\":30}");
}

// GET /ir/data
void handleGetData() {
    if (signalBuffer.empty()) {
        if(isLearningMode) {
             server.send(200, "application/json", "[]");
        } else {
             server.send(204, "application/json", ""); 
        }
        return;
    }

    // JSON Olusturma (ArduinoJson v7 uyumlu)
    JsonDocument doc; // v7'de boyut otomatik veya dinamik

    JsonArray root = doc.to<JsonArray>();

    for (const auto& s : signalBuffer) {
        JsonObject obj = root.add<JsonObject>();
        obj["protocol"] = s.protocolName;
        obj["protocol_id"] = (int)s.protocolId;
        
        obj["value"] = (uint32_t)s.value;
        obj["bits"] = s.bits;
        obj["has_raw"] = s.hasRaw;

        if (s.hasRaw) {
             JsonArray rawArr = obj["raw_data"].to<JsonArray>();
             for(uint16_t r : s.rawData) {
                 rawArr.add(r);
             }
        }
    }

    String jsonOutput;
    serializeJson(doc, jsonOutput);

    server.send(200, "application/json", jsonOutput);

    Serial.println(">> Veri Listesi API ile cekildi. Buffer temizleniyor.");
    clearSignalBuffer();
    stopLearningMode(); 
}

// POST /ir/send
void handleSend() {
    if (!server.hasArg("plain")) {
        server.send(400, "text/plain", "Body missing");
        return;
    }

    String body = server.arg("plain");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, body);

    if (error) {
        server.send(400, "text/plain", "Invalid JSON");
        return;
    }

    Serial.println(">> API'den Gonderme Istegi Geldi...");

    // RAW veri oncelikli
    if (doc["raw_data"].is<JsonArray>()) {
        JsonArray rawArray = doc["raw_data"];
        uint16_t len = rawArray.size();
        uint16_t *rawBuffer = new uint16_t[len];
        
        for (uint16_t i = 0; i < len; i++) {
            rawBuffer[i] = rawArray[i];
        }
        
        uint16_t freq = 38; 
        if (doc["freq"].is<uint16_t>()) freq = doc["freq"];

        Serial.printf(">> RAW Gonderme Basliyor (%d eleman)...\n", len);
        
        // Wrapper uzerinden gonder
        irSender.sendRaw(rawBuffer, len, freq);
        
        delete[] rawBuffer;
        
    } else if (doc["protocol_id"].is<int>() && doc["value"].is<uint32_t>()) {
        int protocol = doc["protocol_id"];
        uint64_t value = doc["value"];
        int bits = doc["bits"]; 

        Serial.printf(">> Protokol (%d) Deger (%u) Gonderiliyor...\n", protocol, (uint32_t)value);
        
        irSender.send((decode_type_t)protocol, value, bits);
        
    } else {
        server.send(400, "text/plain", "Missing raw_data OR protocol/value");
        return;
    }

    server.send(200, "application/json", "{\"status\":\"Signal Sent\"}");
}

// --- Setup ---
void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n\n--- IR Gateway Sistemi Baslatildi ---");

    button.begin();
    statusLed.begin();
    irSender.begin();
    wifiManager.begin();

    server.on("/", HTTP_GET, handleRoot);
    server.on("/status", HTTP_GET, handleStatus);
    server.on("/wifi/config", HTTP_POST, handleWifiConfig);
    server.on("/ir/learn", HTTP_GET, handleStartLearn); 
    server.on("/ir/data", HTTP_GET, handleGetData);
    server.on("/ir/send", HTTP_POST, handleSend);

    server.begin();
    Serial.println("HTTP Sunucu Baslatildi.");

    irReader.begin();
    Serial.println("Sistem Hazir.");
}

// --- Loop ---
void loop() {
    server.handleClient();
    wifiManager.loop();
    unsigned long currentMillis = millis();

    if (button.isLongPressed()) {
        if (!isLearningMode) startLearningMode();
    }

    if (button.isTripleClicked()) {
        if (isLearningMode) {
            Serial.println(">> Manuel Iptal.");
            stopLearningMode();
        }
    }

    if (isLearningMode) {
        statusLed.blink(500, 1000);

        if (currentMillis - learningTimer > LEARN_MODE_TIMEOUT) {
            Serial.println(">> Zaman asimi!");
            stopLearningMode();
        }

        if (irReader.loop()) {
            Serial.println(">> Sinyal Algilandi!");
            learningTimer = currentMillis;
            decode_results results = irReader.getResults();
            addSignalToBuffer(&results);
            irReader.resume();
        }
    } else {
        statusLed.off();
    }
}
