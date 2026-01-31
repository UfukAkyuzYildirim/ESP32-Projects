#include <Arduino.h>
#include <SPI.h>
#include "Joystick.h"
#include "DefaultEspAntenna.h"

// Joystick
#define JOY_X    4 
#define JOY_Y    5 
#define JOY_SW   6 

// ESP-NOW icin karsi tarafin (FlightControl S3) MAC adresi
// Log: E8:F6:0A:8A:D8:38 (COM14)
const uint8_t RECEIVER_MAC[6] = {0xE8, 0xF6, 0x0A, 0x8A, 0xD8, 0x38};

DefaultEspAntenna espNowAntenna(RECEIVER_MAC, true); // true = gonderici
Joystick myJoy(JOY_X, JOY_Y, JOY_SW);

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("--- KUMANDA (Gonderici - ESP32 S3, ESP-NOW) ---");

  myJoy.begin();

  if (espNowAntenna.begin()) {
    Serial.println("✅ ESP-NOW hazir (Gonderici)");
  } else {
    Serial.println("❌ ESP-NOW baslamadi");
    while(1);
  }
}

void loop() {
  int xDegeri = myJoy.getX();
  byte dataToSend = map(xDegeri, -1000, 1000, 0, 255);
  
  Serial.print("Joy: ");
  Serial.print(xDegeri);
    Serial.print(" -> Paket: ");
    Serial.print(dataToSend);

    bool sonuc = espNowAntenna.sendByte(dataToSend);
    Serial.println(sonuc ? " -> [ILETILDI] ✅" : " -> [BASARISIZ] ❌");

  delay(100);
}