#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <esp_now.h>
#include <WiFi.h>
#include "Joystick.h"
#include "DefaultEspAntenna.h"

// --- AYARLAR ---
#define PIN_BTN   4
#define PIN_POT_X 5
#define PIN_POT_Y 6

#define LED_PIN   48
#define LED_COUNT 1

// --- NESNELER ---
Joystick joy1(PIN_POT_X, PIN_POT_Y, PIN_BTN);

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
DefaultEspAntenna radio(broadcastAddress, true); 
Adafruit_NeoPixel rgb(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

typedef struct DronePacket {
    int x;  
    int y;  
    int sw; 
} DronePacket;

DronePacket myData;

void setup() {
    Serial.begin(115200);
    
    // 1. Donanımları Başlat
    joy1.begin();
    rgb.begin();
    rgb.setBrightness(50);
    rgb.show(); // Sönük başla

    // -----------------------------------------------------------
    // 2. KRİTİK DEĞİŞİKLİK: ÖNCE RADYOYU AÇIYORUZ
    // WiFi açılınca voltaj düşer, ADC değerleri değişir.
    // Kalibrasyonu bu düşüşten SONRA yapmalıyız.
    // -----------------------------------------------------------
    Serial.println("Radyo (WiFi) Baslatiliyor...");
    if (radio.begin()) {
        Serial.println("✅ ESP-NOW Basarili.");
    } else {
        Serial.println("❌ Radyo Hatasi!");
        rgb.setPixelColor(0, 255, 0, 0); // Kırmızı
        rgb.show();
        while(1);
    }

    // Voltajın oturması için biraz bekle (Safety Delay)
    delay(500);

    // -----------------------------------------------------------
    // 3. ŞİMDİ KALİBRASYON (Voltaj oturmuş halde)
    // -----------------------------------------------------------
    Serial.println("Kalibrasyon Başlıyor... Kollara Dokunma!");
    rgb.setPixelColor(0, 255, 0, 0); // KIRMIZI (Dokunma!)
    rgb.show();
    
    // 10 Saniyelik Tarama
    joy1.calibrate();

    // -----------------------------------------------------------
    // 4. BAŞARI ANİMASYONU
    // -----------------------------------------------------------
    for(int i=0; i<3; i++) {
        rgb.setPixelColor(0, 0, 255, 0); // YEŞİL
        rgb.show();
        delay(200);
        rgb.setPixelColor(0, 0, 0, 0);   // SÖNÜK
        rgb.show();
        delay(200);
    }

    // -----------------------------------------------------------
    // 5. HAZIR (YEŞİL SABİT)
    // -----------------------------------------------------------
    rgb.setPixelColor(0, 0, 255, 0); // YEŞİL SABİT
    rgb.show();

    Serial.println("--- KUMANDA HAZIR (VOLTAJ STABIL) ---");
}

void loop() {
    myData.x = joy1.getX();
    myData.y = joy1.getY();
    myData.sw = joy1.getToggleState() ? 1 : 0;

    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

    // LED Durumu (Loop içinde)
    if (result == ESP_OK) {
        if (myData.sw == 1) {
            rgb.setPixelColor(0, 50, 0, 0); // ARM: Kırmızı
        } else {
            rgb.setPixelColor(0, 0, 10, 0); // DISARM: Yeşil
        }
    } else {
        rgb.setPixelColor(0, 0, 0, 0); 
    }
    rgb.show();

    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 200) {
        lastPrint = millis();
        joy1.printDebug();
    }
    
    delay(4); 
}