#include <Arduino.h>
#include "Joystick.h"
#include "Antenna.h"

#define JOYSTICK_X_PIN 34
#define JOYSTICK_Y_PIN 35
#define JOYSTICK_SW_PIN 32

#define ANTENNA_CE_PIN 4
#define ANTENNA_CSN_PIN 5

Joystick myJoystick(JOYSTICK_X_PIN, JOYSTICK_Y_PIN, JOYSTICK_SW_PIN);
Antenna myAntenna(ANTENNA_CE_PIN, ANTENNA_CSN_PIN);

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("--- SYSTEM INITIALIZING ---");

    // Initialize joystick inputs
    myJoystick.begin();
    Serial.println("Joystick: OK");

    Serial.println("Probing RF module...");
    if (myAntenna.begin()) {
        Serial.println("✅ RF module detected");
        myAntenna.showDetails();
    } else {
        Serial.println("❌ RF module failure, check wiring");
        while (true) {
            delay(1000);
        }
    }
}

void loop() {
    myJoystick.printDebug();
    delay(1000);
}