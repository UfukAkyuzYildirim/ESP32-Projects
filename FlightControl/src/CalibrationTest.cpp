#include "CalibrationTest.h"
#include "DroneConfig.h"

namespace {
void writeAll(Servo &mFL, Servo &mFR, Servo &mRL, Servo &mRR, int pwm) {
    mFL.writeMicroseconds(pwm);
    mFR.writeMicroseconds(pwm);
    mRL.writeMicroseconds(pwm);
    mRR.writeMicroseconds(pwm);
}
}

void CalibrationTest::setup(Servo &mFL, Servo &mFR, Servo &mRL, Servo &mRR, RadioSystem &radio) {
    Serial.begin(115200);

    mFL.attach(PIN_FL, 1000, 2000);
    mFR.attach(PIN_FR, 1000, 2000);
    mRL.attach(PIN_RL, 1000, 2000);
    mRR.attach(PIN_RR, 1000, 2000);

    writeAll(mFL, mFR, mRL, mRR, 1000);

    Serial.println("--- MANUEL JOYSTICK KALIBRASYON MODU ---");
    Serial.println("1. Joystick YUKARI itili kalsin -> Pili Tak.");
    Serial.println("2. 'Bip-Bip' duyunca -> Joystick ASAGI cek.");

    if (!radio.begin()) {
        Serial.println("❌ Radyo Yok! Kontrol edemezsin.");
    } else {
        Serial.println("✅ Radyo Bagli. Hazir.");
    }
}

void CalibrationTest::loop(Servo &mFL, Servo &mFR, Servo &mRL, Servo &mRR, RadioSystem &radio) {
    if (!radio.isConnectionAlive()) {
        writeAll(mFL, mFR, mRL, mRR, 1000);
        return;
    }

    int joyY = radio.getRawY();

    if (joyY > 900) {
        writeAll(mFL, mFR, mRL, mRR, 2000);
        if (millis() % 500 == 0) Serial.println("YUKARI: 2000 (MAX) Gonderiliyor...");
    } else if (joyY < 100) {
        writeAll(mFL, mFR, mRL, mRR, 1000);
        if (millis() % 500 == 0) Serial.println("ASAGI: 1000 (MIN) Gonderiliyor...");
    } else {
        writeAll(mFL, mFR, mRL, mRR, 1000);
    }

    delay(10);
}
