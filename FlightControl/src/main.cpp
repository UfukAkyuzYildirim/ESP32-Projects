#include <Arduino.h>
#include <ESP32Servo.h>
#include "RadioSystem.h"
#include "DroneConfig.h"

// --- SENİN DOĞRU PİNLERİN ---
#define PIN_FL 4   // Sol Ön
#define PIN_FR 17  // Sağ Ön
#define PIN_RL 13  // Sol Arka
#define PIN_RR 21  // Sağ Arka

Servo motFL, motFR, motRL, motRR;
RadioSystem radio;
float currentPwm = 1000.0; 
unsigned long lastTime = 0;
unsigned long lastLogTime = 0;

void setup() {
    Serial.begin(115200);
    
    // 1. Radyoyu Başlat
    if (!radio.begin()) {
        Serial.println("❌ Radyo Hatasi!");
        while(1);
    }
    Serial.println("✅ Radyo Hazır.");

    motFL.setPeriodHertz(50);
    motFR.setPeriodHertz(50);
    motRL.setPeriodHertz(50);
    motRR.setPeriodHertz(50);

    motFL.attach(PIN_FL, 1000, 2000);
    motFR.attach(PIN_FR, 1000, 2000);
    motRL.attach(PIN_RL, 1000, 2000);
    motRR.attach(PIN_RR, 1000, 2000);
    
    motFL.writeMicroseconds(1000);
    motFR.writeMicroseconds(1000);
    motRL.writeMicroseconds(1000);
    motRR.writeMicroseconds(1000);
    
    delay(2000); 
    Serial.println("✅ Motorlar Hazır. Pili Tak, Switch'i Aç!");
}

void loop() {
    unsigned long now = millis();
    float dt = (now - lastTime) / 1000.0;
    lastTime = now;

    if (!radio.isConnectionAlive() || !radio.isSwitchOn()) {
        currentPwm = 1000;
        
        motFL.writeMicroseconds(1000);
        motFR.writeMicroseconds(1000);
        motRL.writeMicroseconds(1000);
        motRR.writeMicroseconds(1000);

        if (millis() - lastLogTime > 1000) {
            Serial.println("🔒 DISARMED: Motorlar Kilitli.");
            lastLogTime = millis();
        }
        return;
    }

    int liftSpeed = radio.getLiftSpeed(); // Joystick'ten gelen veri (-500..+500)

    // Eğer joystick oynuyorsa gazı değiştir
    if (abs(liftSpeed) > 0) {
        currentPwm += liftSpeed * 2.5 * dt; 
    }

    // Sınırlandırma (Şimdilik 1500 ile sınırlayalım)
    if (currentPwm < 1000) currentPwm = 1000;
    if (currentPwm > 1500) currentPwm = 1500; 

    motFL.writeMicroseconds((int)currentPwm);
    motFR.writeMicroseconds((int)currentPwm);
    motRL.writeMicroseconds((int)currentPwm);
    motRR.writeMicroseconds((int)currentPwm);

    if (millis() - lastLogTime > 500) {
        Serial.printf("ARMED! Gaz: %d | Joystick: %d\n", (int)currentPwm, liftSpeed);
        lastLogTime = millis();
    }
}