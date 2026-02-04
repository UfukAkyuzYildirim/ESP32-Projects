#ifndef MOTOR_SYSTEM_H
#define MOTOR_SYSTEM_H

#include <Arduino.h>
#include <ESP32Servo.h>
#include "DroneConfig.h"

class MotorSystem {
private:
    Servo mFR, mFL, mRL, mRR;
    bool armed;

public:
    MotorSystem();
    void begin();
    void arm();
    void disarm();
    bool isArmed();
    void writeAll(int pwm);
    void writeMixed(int fl, int fr, int rl, int rr);
};

#endif