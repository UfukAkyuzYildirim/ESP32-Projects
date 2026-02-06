#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "Joystick.h"
#include "DefaultEspAntenna.h"

// --- PIN AYARLARI ---
#define PIN_BTN   4
#define PIN_POT_X 5
#define PIN_POT_Y 6

#define LED_PIN   48
#define LED_COUNT 1

// --- NESNELER ---
Joystick joy1(PIN_POT_X, PIN_POT_Y, PIN_BTN);

// Drone Broadcast Adresi
const uint8_t droneMac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
DefaultEspAntenna radio(droneMac, true); // true = Sender

Adafruit_NeoPixel rgb(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// Paket Yapisi
typedef struct DronePacket {
    int x;  // -500..+500
    int y;  // -500..+500
    int sw; // 0/1
} DronePacket;

DronePacket myData;

void setup() {
    Serial.begin(115200);
    
    joy1.begin();

    rgb.begin();
    rgb.setBrightness(50);
    rgb.show();

    if (radio.begin()) {
        rgb.setPixelColor(0, 0, 0, 255); // Mavi
        rgb.show();
    } else {
        Serial.println("❌ Radyo Hatasi!");
        rgb.setPixelColor(0, 255, 0, 0); // Kirmizi
        rgb.show();
        while(1);
    }
}

void loop() {
    // 1. Verileri Hazirla
    myData.x = joy1.getX();
    myData.y = joy1.getY();
    myData.sw = joy1.isPressed() ? 1 : 0;

    // 2. Gonder (Hizli - 50Hz)
    bool success = radio.send(&myData, sizeof(myData));

    // 3. LED Durumu
    if (success) rgb.setPixelColor(0, 0, 10, 0); // Yesil
    else rgb.setPixelColor(0, 50, 0, 0);         // Kirmizi
    rgb.show();

    delay(20); // Dongu hizi
}