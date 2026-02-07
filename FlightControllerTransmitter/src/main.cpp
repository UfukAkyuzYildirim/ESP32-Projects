#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <esp_now.h>
#include <WiFi.h>
#include "Joystick.h"
#include "DefaultEspAntenna.h"

// --- AYARLAR ---
#define PIN_BTN_L  4  // Sol Buton Pin 4
#define PIN_BTN_R  9  // Sağ Buton Pin 9

#define PIN_POT_LX 5 // Sol Joystick X Pin
#define PIN_POT_LY 6 // Sol Joystick Y Pin

#define PIN_POT_RX 7 // Sağ Joystick X Pin
#define PIN_POT_RY 8 // Sağ Joystick Y Pin

#define LED_PIN   48
#define LED_COUNT 1

Joystick joy1(PIN_POT_LX, PIN_POT_LY, PIN_POT_RX, PIN_POT_RY, PIN_BTN_L, PIN_BTN_R);

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
DefaultEspAntenna radio(broadcastAddress, true); 
Adafruit_NeoPixel rgb(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

typedef struct DronePacket {
    int lx; int ly;
    int rx; int ry;
    int sw; 
} DronePacket;

DronePacket myData;

void setup() {
    Serial.begin(115200);
    joy1.begin();
    rgb.begin(); rgb.setBrightness(50); rgb.show(); 

    Serial.println("Radyo Baslatiliyor...");
    if (radio.begin()) Serial.println("✅ OK");
    else { Serial.println("❌ FAIL"); while(1); }
    delay(500);

    Serial.println("Kalibrasyon (3 Sn)... DOKUNMA!");
    rgb.setPixelColor(0, 255, 0, 0); rgb.show();
    joy1.calibrate();

    rgb.setPixelColor(0, 0, 255, 0); rgb.show();
    Serial.println("--- KUMANDA HAZIR ---");
}

void loop() {
    myData.lx = joy1.getLX();
    myData.ly = joy1.getLY();
    myData.rx = joy1.getRX();
    myData.ry = joy1.getRY();
    myData.sw = joy1.getToggleState() ? 1 : 0;

    esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

    if(myData.sw) rgb.setPixelColor(0, 50, 0, 0);
    else rgb.setPixelColor(0, 0, 10, 0);
    rgb.show();

    static unsigned long t = 0;
    if (millis() - t > 200) {
        t = millis();
        joy1.printDebug();
    }
    delay(4); 
}