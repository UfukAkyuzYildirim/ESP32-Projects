#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

#include <Arduino.h>

class PidController {
private:
    float kp, ki, kd;
    float previousError;
    float integral;
    float maxIntegral;

public:
    PidController(float p, float i, float d, float maxI);
    float compute(float setpoint, float measuredValue, float dt);
    void reset();
};

#endif