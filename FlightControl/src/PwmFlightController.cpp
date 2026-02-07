#include "PwmFlightController.h"

PwmFlightController::PwmFlightController(MotorSystem &motorsRef, RadioSystem &radioRef)
    : motors(motorsRef), radio(radioRef), currentPwm(1000.0f), lastTime(0), lastLogTime(0) {}

bool PwmFlightController::begin() {
    if (!radio.begin()) {
        Serial.println("❌ Radyo Hatasi!");
        return false;
    }

    motors.begin();
    motors.writeAll(MIN_PULSE);
    delay(500);
    Serial.println("✅ PWM ESC modu hazir");
    return true;
}

void PwmFlightController::loopStep() {
    unsigned long now = millis();
    float dt = (now - lastTime) / 1000.0f;
    lastTime = now;

    if (!radio.isConnectionAlive() || !radio.isSwitchOn()) {
        currentPwm = MIN_PULSE;
        motors.disarm();
        motors.writeAll(MIN_PULSE);
        if (millis() - lastLogTime > 1000) {
            Serial.println("🔒 PWM DISARM");
            lastLogTime = millis();
        }
        return;
    }

    int liftSpeed = radio.getLiftSpeed();
    if (abs(liftSpeed) > 0) {
        currentPwm += liftSpeed * 2.5f * dt;
    }

    if (currentPwm < MIN_PULSE) currentPwm = MIN_PULSE;
    if (currentPwm > 1500) currentPwm = 1500;

    motors.arm();
    motors.writeAll(static_cast<int>(currentPwm));

    if (millis() - lastLogTime > 500) {
        Serial.printf("[PWM] Gaz: %d | Joy: %d\n", static_cast<int>(currentPwm), liftSpeed);
        lastLogTime = millis();
    }
}
