#include <Arduino.h>

// NodeMCU D1 Pini = GPIO 5
const uint16_t kIrLed = 5;

void setup() {
  // Pini cikis olarak ayarla
  pinMode(kIrLed, OUTPUT);
  Serial.begin(115200);
  Serial.println("Donanim Testi: LED 3 saniye YANACAK, 2 saniye SONECEK.");
}

void loop() {
  Serial.println("LED YANIYOR (Kamerayi tut)...");
  digitalWrite(kIrLed, HIGH); // LED'e surekli 3.3V ver
  delay(3000); // 3 saniye bekle (Kamerayla bakman icin sure)

  Serial.println("LED SONDU.");
  digitalWrite(kIrLed, LOW);  // LED'i kapat
  delay(2000); // 2 saniye bekle
}
