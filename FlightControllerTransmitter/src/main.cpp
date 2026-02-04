#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <ESP32Servo.h>
#include <esp_now.h>
#include <WiFi.h>

#define PIN_FR 13 
#define PIN_FL 16 
#define PIN_RL 14 
#define PIN_RR 15 

#define MIN_PULSE 1000   
#define MAX_PULSE 2000   

Servo mFR, mFL, mRL, mRR;

typedef struct struct_message {
  int x;  
  int y;  
  int sw; 
} struct_message;

struct_message myData;
unsigned long lastRecvTime = 0;

bool isArmed = false;     
int lastButtonState = 0;   

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  lastRecvTime = millis();
}

void setup() {
  Serial.begin(115200);
  
  mFL.attach(PIN_FL, MIN_PULSE, MAX_PULSE);
  mFR.attach(PIN_FR, MIN_PULSE, MAX_PULSE);
  mRL.attach(PIN_RL, MIN_PULSE, MAX_PULSE);
  mRR.attach(PIN_RR, MIN_PULSE, MAX_PULSE);

  mFL.writeMicroseconds(MIN_PULSE);
  mFR.writeMicroseconds(MIN_PULSE);
  mRL.writeMicroseconds(MIN_PULSE);
  mRR.writeMicroseconds(MIN_PULSE);
  
  delay(2000); 

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) return;
  esp_now_register_recv_cb(OnDataRecv);

  Serial.println("✅ KALIBRASYON MODU HAZIR!");
}

void loop() {
  if (millis() - lastRecvTime > 1000) {
    isArmed = false;
    mFL.writeMicroseconds(MIN_PULSE);
    mFR.writeMicroseconds(MIN_PULSE);
    mRL.writeMicroseconds(MIN_PULSE);
    mRR.writeMicroseconds(MIN_PULSE);
    return;
  }

  if (myData.sw == 1 && lastButtonState == 0) {
      isArmed = !isArmed;
      if (isArmed) Serial.println("⚠️ MOTORLAR AKTIF! Dikkatli ol...");
      else Serial.println("⛔ MOTORLAR KAPANDI");
  }
  lastButtonState = myData.sw; 

  int pwmSignal = MIN_PULSE; // Varsayilan 1000

  if (isArmed) {
      pwmSignal = map(myData.y, 0, 1000, 1000, 1500);
  }

  mFL.writeMicroseconds(pwmSignal);
  mFR.writeMicroseconds(pwmSignal);
  mRL.writeMicroseconds(pwmSignal);
  mRR.writeMicroseconds(pwmSignal);

  if (isArmed && pwmSignal > 1010) {
      Serial.print("MOTORA GIDEN PWM SINYALI: ");
      Serial.println(pwmSignal);
  }
  
  delay(50); 
}