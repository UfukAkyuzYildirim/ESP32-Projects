#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include <Arduino.h>
#include <ESP32Servo.h>

class MotorController {
public:
    MotorController(int pin, int minUs = 1000, int maxUs = 2000, float maxFactor = 0.5f);

    void begin();

    void writeByte(uint8_t value);
    void stop();

private:
    int _pin;
    int _minUs;
    int _maxUs;
    float _maxFactor;
    Servo _servo;
};

#endif
