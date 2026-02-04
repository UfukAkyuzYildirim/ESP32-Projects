#include <Arduino.h>
#include <ESP32Servo.h> 
#include "DroneConfig.h" 
#include "RadioSystem.h"
#include <SPI.h> 
#include <Wire.h>
#include "MotorSystem.h"
#include "ImuSystem.h"


Servo mFL;
Servo mFR;
Servo mRL;
Servo mRR;

RadioSystem radio;

int currentPWM = 1000;
bool isStickReady = true;

void writeAll(int pwm) {
    mFL.writeMicroseconds(pwm);
    mFR.writeMicroseconds(pwm);
    mRL.writeMicroseconds(pwm);
    mRR.writeMicroseconds(pwm);
}

ImuSystem imuSystem;

static constexpr float PITCH_THRESHOLD = 5.0f;
static constexpr float ROLL_THRESHOLD  = 5.0f;

String lastStatus = "";

void printStatus(const String &msg) {
    if (msg != lastStatus) {
        Serial.println(msg);
        lastStatus = msg;
    }
}

void ImuTestSetup() {
    Serial.begin(115200);
    delay(300);
    Serial.println("--- BNO055 YATIS TESTI ---");
    Serial.println("SDA=GPIO1, SCL=GPIO2 bagli oldugunu teyit et.");

    if (!imuSystem.begin()) {
        Serial.println("❌ BNO055 baslamadi, baglantilari kontrol et.");
        while (1) {
            delay(1000);
        }
    }

    Serial.println("✅ Sensor hazir. Dronu oynat, durum mesaji gelsin.");
}

void ImuTestLoop() {
    DroneAngles ang = imuSystem.getAngles();

    if (ang.pitch > PITCH_THRESHOLD) {
        printStatus("Burun kalkti");
    } else if (ang.pitch < -PITCH_THRESHOLD) {
        printStatus("Arkasi kalkti");
    }
    else if (ang.roll > ROLL_THRESHOLD) {
        printStatus("Sol taraf kalkti");
    } else if (ang.roll < -ROLL_THRESHOLD) {
        printStatus("Sag taraf kalkti");
    } else {
        printStatus("Duz (degisim yok)");
    }

    delay(200);
}


void CalibrationSetup(){
    Serial.begin(115200);
    
    mFL.attach(PIN_FL, 1000, 2000);
    mFR.attach(PIN_FR, 1000, 2000);
    mRL.attach(PIN_RL, 1000, 2000);
    mRR.attach(PIN_RR, 1000, 2000);

    writeAll(1000);

    Serial.println("--- MANUEL JOYSTICK KALIBRASYON MODU ---");
    Serial.println("1. Joystick YUKARI itili kalsin -> Pili Tak.");
    Serial.println("2. 'Bip-Bip' duyunca -> Joystick ASAGI cek.");
    
    if (!radio.begin()) {
        Serial.println("❌ Radyo Yok! Kontrol edemezsin.");
    } else {
        Serial.println("✅ Radyo Bagli. Hazir.");
    }
}
void IncrementTestSetup(){
    Serial.begin(115200);
    
    mFL.attach(PIN_FL, 1000, 2000);
    mFR.attach(PIN_FR, 1000, 2000);
    mRL.attach(PIN_RL, 1000, 2000);
    mRR.attach(PIN_RR, 1000, 2000);

    writeAll(1000);

    Serial.println("--- 4 MOTOR HASSAS TEST (STEPPER MODU) ---");
    
    if (!radio.begin()) {
        Serial.println("❌ Radyo Yok! Baglantiyi kontrol et.");
    } else {
        Serial.println("✅ Radyo Bagli. Teste baslayabilirsin.");
    }
    
    Serial.print("Baslangic PWM: ");
    Serial.println(currentPWM);
}

void setup() {
    IncrementTestSetup();
}

void CalibrationLoop(){
    if (!radio.isConnectionAlive()) {
        writeAll(1000);
        return;
    }

    int joyY = radio.getRawY();

    if (joyY > 900) {
        writeAll(2000); 
        if(millis() % 500 == 0) Serial.println("YUKARI: 2000 (MAX) Gonderiliyor...");
    }
    else if (joyY < 100) {
        writeAll(1000); 
        if(millis() % 500 == 0) Serial.println("ASAGI: 1000 (MIN) Gonderiliyor...");
    }
    else {
        writeAll(1000);
    }
    
    delay(10);
}
void IncrementTestLoop(){
    if (!radio.isConnectionAlive()) {
        writeAll(1000);
        return;
    }

    int joyY = radio.getRawY(); 

    if (joyY > 900 && isStickReady) {
        currentPWM++;
        if (currentPWM > 2000) currentPWM = 2000;
        
        isStickReady = false;
        
        Serial.print(">>> ARTTI (+1) -> PWM: "); 
        Serial.println(currentPWM);
    }
    else if (joyY < 100 && isStickReady) {
        currentPWM--;
        if (currentPWM < 1000) currentPWM = 1000;
        
        isStickReady = false; 
        
        Serial.print("<<< AZALDI (-1) -> PWM: "); 
        Serial.println(currentPWM);
    }
    else if (joyY > 400 && joyY < 600) {
        isStickReady = true;
    }

    writeAll(currentPWM);

    delay(10);
}

void loop() {
    IncrementTestLoop();
}




