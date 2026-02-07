#include <Arduino.h>
#include "DroneConfig.h"
#include "RadioSystem.h"
#include "DShotMotorSystem.h"   
#include "DShotFlightController.h"
#include "ImuSystem.h" 

// --- OBJECTS ---
RadioSystem radioSystem;
DShotMotorSystem dshotMotors;
ImuSystem droneImu; // Sensor Object

// FlightController now requires 3 parameters: Motors, Radio, and SENSOR.
DShotFlightController dshotCtrl(dshotMotors, radioSystem, droneImu);

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("--- SYSTEM STARTUP (PID FLIGHT MODE) ---");

    // 1. INITIALIZE SENSOR
    // Prevent flight if sensor fails
    Serial.print("IMU (BNO055)... ");
    if (droneImu.begin()) {
        Serial.println("OK");
    } else {
        Serial.println("ERROR (Check wiring: SDA->1, SCL->2)");
        while(1); // Halt on error
    }

    // 2. INITIALIZE FLIGHT CONTROLLER
    // Internally initializes Radio and DShot Motors
    Serial.print("Flight Controller... ");
    if (dshotCtrl.begin()) {
        Serial.println("OK");
    } else {
        Serial.println("ERROR (Radio or Motor failure)");
        while(1);
    }

    Serial.println("SYSTEM READY. Arm via controller.");
}

void loop() {
    // All logic (Sensor read, PID calc, Motor drive) is handled here.
    dshotCtrl.loopStep();
}