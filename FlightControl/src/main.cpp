#include <Arduino.h>
#include "DefaultEspAntenna.h"

// ESP-NOW icin gondericinin (Kumanda S3) MAC adresi
// Loglarda S3 MAC: E8:F6:0A:88:F2:84
const uint8_t SENDER_MAC[6] = {0xE8, 0xF6, 0x0A, 0x88, 0xF2, 0x84};

DefaultEspAntenna espNowAntenna(SENDER_MAC, false); // false = alici

void setup() {
  Serial.begin(115200);
  Serial.println("--- UCUS KARTI (ALICI - ESP-NOW) ---");
  
  if (espNowAntenna.begin()) {
    Serial.println("✅ ESP-NOW hazir (Alici)");
  } else {
    Serial.println("❌ ESP-NOW baslamadi");
    while(1);
  }
}

void loop() {
  // Havada paket var mi?
  if (espNowAntenna.isAvailable()) {
    byte receivedData = espNowAntenna.readByte();
    Serial.print("📡 Gelen Veri: ");
    Serial.println(receivedData);
  }
  
  // ESP'nin donmadigini gormek icin her 1 saniyede nokta koyalim
  static unsigned long lastTime = 0;
  if (millis() - lastTime > 1000) {
      Serial.print("."); 
      lastTime = millis();
  }
}