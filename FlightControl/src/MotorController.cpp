#include "MotorController.h"

MotorController::MotorController(int pin, int minUs, int maxUs, float maxFactor)
    : _pin(pin), _minUs(minUs), _maxUs(maxUs), _maxFactor(maxFactor) {}

void MotorController::begin() {
    _servo.attach(_pin, _minUs, _maxUs);
    stop();
}

void MotorController::writeByte(uint8_t value) {
    int span = _maxUs - _minUs;
    int cappedSpan = static_cast<int>(span * _maxFactor);
    int target = map(value, 0, 255, _minUs, _minUs + cappedSpan);
    _servo.writeMicroseconds(target);
}

void MotorController::stop() {
    _servo.writeMicroseconds(_minUs);
}
