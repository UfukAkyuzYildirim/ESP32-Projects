#include "DShotFlightController.h"

DShotFlightController::DShotFlightController(DShotMotorSystem &motorsRef, RadioSystem &radioRef)
    : motors(motorsRef), radio(radioRef), currentUs(MIN_PULSE), lastTime(0), lastLogTime(0) {}

bool DShotFlightController::begin() {
    if (!radio.begin()) {
        Serial.println("❌ Radyo Hatasi (DShot)");
        return false;
    }
    if (!motors.begin()) {
        Serial.println("❌ DShot motor baslatilamadi");
        return false;
    }
    motors.writeAllUs(MIN_PULSE);
    delay(500);
    Serial.println("✅ DShot modu hazir");
    return true;
}

void DShotFlightController::loopStep() {
    unsigned long now = millis();
    float dt = (now - lastTime) / 1000.0f;
    lastTime = now;

    if (!radio.isConnectionAlive() || !radio.isSwitchOn()) {
        currentUs = MIN_PULSE;
        motors.disarm();
        if (millis() - lastLogTime > 1000) {
            Serial.println("🔒 DShot DISARM");
            lastLogTime = millis();
        }
        return;
    }

    int liftSpeed = radio.getLiftSpeed();
    if (abs(liftSpeed) > 0) {
        currentUs += liftSpeed * 2.5f * dt;
    }

    if (currentUs < MIN_PULSE) currentUs = MIN_PULSE;
    if (currentUs > 1500) currentUs = 1500;

    motors.writeAllUs(static_cast<int>(currentUs));

    if (millis() - lastLogTime > 500) {
        Serial.printf("[DSHOT] Gaz(us): %d | Joy: %d\n", static_cast<int>(currentUs), liftSpeed);
        lastLogTime = millis();
    }
}
