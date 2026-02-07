#include <Arduino.h>
#include "DroneConfig.h"
#include "RadioSystem.h"
#include "MotorSystem.h"        // PWM Motor Sistemi
#include "PwmFlightController.h"
#include "DShotMotorSystem.h"   // DShot Motor Sistemi
#include "DShotFlightController.h"

// --- SETTINGS ---
#define USE_DSHOT 1  // 1: DShot (Digital), 0: PWM (Analog)

// --- SHARED OBJECTS ---
// Radio system is shared, single instance.
RadioSystem radioSystem;

// --- CONTROLLER SELECTION ---
// Using global pointers for memory management
DShotMotorSystem* dshotMotors = nullptr;
DShotFlightController* dshotCtrl = nullptr;

MotorSystem* pwmMotors = nullptr;
PwmFlightController* pwmCtrl = nullptr;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("--- DRONE INITIALIZING ---");

    if (USE_DSHOT) {
        Serial.println("Mode: DSHOT300 (Digital)");
        
        // Create DShot Objects
        dshotMotors = new DShotMotorSystem();
        dshotCtrl = new DShotFlightController(*dshotMotors, radioSystem);
        
        if (!dshotCtrl->begin()) {
            Serial.println("❌ CRITICAL ERROR: DShot Failed!");
            while (1) { delay(100); }
        }

    } else {
        Serial.println("Mode: PWM (Analog)");
        
        // Create PWM Objects
        pwmMotors = new MotorSystem();
        pwmCtrl = new PwmFlightController(*pwmMotors, radioSystem);

        if (!pwmCtrl->begin()) {
            Serial.println("❌ CRITICAL ERROR: PWM Failed!");
            while (1) { delay(100); }
        }
    }

    Serial.println("✅ SYSTEM READY. WAITING FOR TRANSMITTER...");
}

void loop() {
    if (USE_DSHOT) {
        dshotCtrl->loopStep();
    } else {
        pwmCtrl->loopStep();
    }
}