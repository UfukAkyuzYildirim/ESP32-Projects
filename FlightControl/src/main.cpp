#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WebSerial.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#include "DroneConfig.h"
#include "RadioSystem.h"
#include "DShotMotorSystem.h"   
#include "DShotFlightController.h"
#include "ImuSystem.h" 

// --- WIFI & WEB ---
const char* ssid = "DRON_HACI";
const char* password = "12345678";
AsyncWebServer server(80);

// --- OBJECTS ---
RadioSystem radioSystem;
DShotMotorSystem dshotMotors;
ImuSystem droneImu; // Sensor Object

// FlightController now requires 3 parameters: Motors, Radio, and SENSOR.
DShotFlightController dshotCtrl(dshotMotors, radioSystem, droneImu);

void setup() {
    // 0. Disable Brownout Detector (CRITICAL for Battery + WiFi)
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 

    Serial.begin(115200);
    delay(1000);
    Serial.println("--- SYSTEM STARTUP (PID FLIGHT MODE) ---");

    // 2. INITIALIZE SENSOR
    // Prevent flight if sensor fails
    Serial.print("IMU (BNO055)... ");
    if (droneImu.begin()) {
        Serial.println("OK");
        WebSerial.println("SENSOR OK!");
    } else {
        Serial.println("ERROR (Check wiring: SDA->1, SCL->2)");
        WebSerial.println("SENSOR ERROR!");
        while(1); // Halt on error
    }

    // 3. INITIALIZE FLIGHT CONTROLLER
    // Internally initializes Radio and DShot Motors
    Serial.print("Flight Controller... ");
    if (dshotCtrl.begin()) {
        Serial.println("OK");
        WebSerial.println("CONTROLLER OK!");
    } else {
        Serial.println("ERROR (Radio or Motor failure)");
        WebSerial.println("CONTROLLER ERROR!");
        while(1);
    }
    // --- WIFI & WEBSERIAL (AFTER CONTROLLER TO PREVENT OVERWRITE) ---
    WiFi.mode(WIFI_AP_STA); // AP + STA (Receiver + WebSerial)
    if(WiFi.softAP(ssid, password)) {
        Serial.print("WiFi OPEN: ");
        Serial.println(WiFi.softAPIP());
    } else {
        Serial.println("WiFi FAIL!");
    }
    
    WebSerial.begin(&server);
    server.begin();
    Serial.println("SYSTEM READY. Arm via controller.");
    WebSerial.println("READY. Waiting for Arming...");
}

void loop() {
    // All logic (Sensor read, PID calc, Motor drive) is handled here.
    dshotCtrl.loopStep();

    // --- WEB LOGGING (Every 1 Second) ---
    static unsigned long lastLog = 0;
    if (millis() - lastLog > 1000) {
        DroneAngles angles = droneImu.getAngles();
        
        // --- 0 Noktasina Gore Aci (Relative Yaw) ---
        // Sag donus: +, Sol donus: -
        float relativeYaw = dshotCtrl.getRelativeYaw();

        String msg = "Pitch: " + String(angles.pitch, 1) + 
                     " | Roll: " + String(angles.roll, 1) + 
                     " | Yaw(Rel): " + String(relativeYaw, 1);
        
        // Opsiyonel: Kumanda verisi (Radio)
        msg += " | Gaz: " + String(radioSystem.getLY()); // Gaz
        
        // --- Motor Güçleri (FL, FR, RL, RR) ---
        msg += " | FL: " + String((int)dshotCtrl.motorFL) + 
               " FR: " + String((int)dshotCtrl.motorFR) + 
               " RL: " + String((int)dshotCtrl.motorRL) + 
               " RR: " + String((int)dshotCtrl.motorRR);
        
        WebSerial.println(msg);
        lastLog = millis();
    }
}