#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

/*
   GPIO (ESP32-DevKitC):
   SDA -> GPIO 21
   SCL -> GPIO 22
*/

// BNO055 sensor instance
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);

void setup(void) {
  Serial.begin(115200);
  
  // Report if the sensor fails to initialize
  if (!bno.begin()) {
    Serial.println("Error: BNO055 sensor not detected. Check wiring.");
    while (1);
  }

  // Use the external crystal oscillator for better stability
  bno.setExtCrystalUse(true);
}

void loop(void) {
  // 1. Get sensor event
  sensors_event_t event;
  bno.getEvent(&event);

  /*
     x = Right-Left (Heading / Yaw)
     y = Up-Down (Pitch)
     z = (Roll)
  */
  float x = event.orientation.x;
  float y = event.orientation.y;
  float z = event.orientation.z;

  // 3. Send to Unity (Format: "ROT:120.50,10.20,-5.00")
  Serial.print("ROT:");
  Serial.print(x);
  Serial.print(",");
  Serial.print(y);
  Serial.print(",");
  Serial.println(z);

  delay(50);
}