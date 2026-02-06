#include "IncrementTest.h"
#include "DroneConfig.h"

namespace {
void writeAll(Servo &mFL, Servo &mFR, Servo &mRL, Servo &mRR, int pwm) {
    mFL.writeMicroseconds(pwm);
    mFR.writeMicroseconds(pwm);
    mRL.writeMicroseconds(pwm);
    mRR.writeMicroseconds(pwm);
}
}

void IncrementTest::setup(Servo &mFL, Servo &mFR, Servo &mRL, Servo &mRR, RadioSystem &radio, int &currentPwm, bool &isStickReady) {
    Serial.begin(115200);

    mFL.attach(PIN_FL, 1000, 2000);
    mFR.attach(PIN_FR, 1000, 2000);
    mRL.attach(PIN_RL, 1000, 2000);
    mRR.attach(PIN_RR, 1000, 2000);

    writeAll(mFL, mFR, mRL, mRR, 1000);

    Serial.println("--- 4 MOTOR HASSAS TEST (STEPPER MODU) ---");

    if (!radio.begin()) {
        Serial.println("❌ Radyo Yok! Baglantiyi kontrol et.");
    } else {
        Serial.println("✅ Radyo Bagli. Teste baslayabilirsin.");
    }

    Serial.print("Baslangic PWM: ");
    Serial.println(currentPwm);

    isStickReady = true;
}

void IncrementTest::loop(Servo &mFL, Servo &mFR, Servo &mRL, Servo &mRR, RadioSystem &radio, int &currentPwm, bool &isStickReady) {
    if (!radio.isConnectionAlive()) {
        writeAll(mFL, mFR, mRL, mRR, 1000);
        return;
    }

    int joyY = radio.getRawY();

    if (joyY > 900 && isStickReady) {
        currentPwm++;
        if (currentPwm > 2000) currentPwm = 2000;
        isStickReady = false;
        Serial.print(">>> ARTTI (+1) -> PWM: ");
        Serial.println(currentPwm);
    } else if (joyY < 100 && isStickReady) {
        currentPwm--;
        if (currentPwm < 1000) currentPwm = 1000;
        isStickReady = false;
        Serial.print("<<< AZALDI (-1) -> PWM: ");
        Serial.println(currentPwm);
    } else if (joyY > 400 && joyY < 600) {
        isStickReady = true;
    }

    writeAll(mFL, mFR, mRL, mRR, currentPwm);
    delay(10);
}
