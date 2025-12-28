#include <Arduino.h>
#include <IRutils.h> 

#include "Config.h"
#include "ButtonHandler.h"
#include "StatusLed.h"
#include "IrReader.h"
#include "IrSender.h"

// --- Global Nesneler ---
ButtonHandler button(PIN_BUTTON);
StatusLed statusLed(PIN_LED);
IrReader irReader(PIN_IR_RECV);
IrSender irSender(PIN_IR_SEND);

// --- Degiskenler ---
decode_results lastResults;      // Son okunan IR kodu
uint16_t *savedRawData = nullptr;// RAW veriyi saklamak icin pointer
uint16_t savedRawLength = 0;     // RAW veri uzunlugu

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
}

// Ogrenme Modunu Baslat
void startLearningMode() {
    if (isLearningMode) return; // Zaten aciksa islem yapma
    
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

// --- Setup ---
void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n\n--- IR Kontrol Sistemi Baslatildi ---");

    button.begin();
    statusLed.begin();
    irSender.begin();
    
    // Baslangicta IR okuyucu da baslasin ama loop icinde sadece mod aktifken okuyacagiz
    irReader.begin();

    Serial.println("Bekleme Modunda...");
}

// --- Loop ---
void loop() {
    unsigned long currentMillis = millis();

    // 1. Buton Kontrolleri
    // ------------------------------------------------
    // Uzun basma kontrolu (3 saniye) -> Ogrenme Modunu Acar
    if (button.isLongPressed()) {
        if (!isLearningMode) {
            startLearningMode();
        }
    }

    // 3 Kere tiklama kontrolu -> Ogrenme Modunu Kapatir
    if (button.isTripleClicked()) {
        if (isLearningMode) {
            Serial.println(">> Manuel Iptal (3 tiklama).");
            stopLearningMode();
        }
    }


    // 2. Ogrenme Modu Mantigi
    // ------------------------------------------------
    if (isLearningMode) {
        // A) LED Yanip Sonsun (0.5 sn YAN, 1 sn SON)
        statusLed.blink(500, 1000);

        // B) Zaman Asimi Kontrolu (30 sn)
        if (currentMillis - learningTimer > LEARN_MODE_TIMEOUT) {
            Serial.println(">> Zaman asimi! (30 sn doldu)");
            stopLearningMode();
        }

        // C) IR Sinyal Okuma
        if (irReader.loop()) {
            Serial.println(">> Sinyal Algilandi!");
            
            // Zaman asimini SIFIRLA (Sureyi uzat)
            learningTimer = currentMillis;

            // Onceki veriyi temizle
            clearSavedData();

            // Yeni veriyi al
            lastResults = irReader.getResults();
            
            // RAW Kopyalama (Derin Kopya)
            savedRawData = resultToRawArray(&lastResults); 
            savedRawLength = getCorrectedRawLength(&lastResults);
            
            Serial.print("Protocol: ");
            Serial.print(typeToString(lastResults.decode_type));
            Serial.print(" Value: ");
            serialPrintUint64(lastResults.value, HEX);
            Serial.println("");
            Serial.println(">> Kod Hafizaya Alindi. (Sure 30 sn uzatildi)");

            // Okumaya devam et
            irReader.resume();
        }
    } else {
        // Mod kapaliyken LED sonuk kalsin
        statusLed.off();
        // Mod kapaliyken IR okumasi yapmiyoruz (irReader.loop() cagirilmiyor)
    }


    // 3. Serial Komutlari (Test Amacli Gonderim)
    // ------------------------------------------------
    if (Serial.available() > 0) {
        char cmd = Serial.read();
        if (cmd == 's' || cmd == 'S') {
            if (savedRawData != nullptr) {
                Serial.println(">> Kayitli Kod Gonderiliyor...");
                
                // UNKNOWN veya RAW veri ise RAW gonder
                if (lastResults.decode_type == UNKNOWN) {
                    IRsend tempSender(PIN_IR_SEND);
                    tempSender.begin();
                    tempSender.sendRaw(savedRawData, savedRawLength, 38); 
                } else {
                    // Tanimli protokol (NEC, Sony vs.)
                    irSender.send(&lastResults);
                }
                
                // Gonderdikten sonra kendi sinyalimizi almayalim diye kisa bekleme ve resume
                delay(100);
                if (isLearningMode) irReader.resume(); 

            } else {
                Serial.println("!! Gonderilecek kod yok. Once ogrenme modunu acip kod tanitin.");
            }
        }
    }
}