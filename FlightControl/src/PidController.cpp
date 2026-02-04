#include "PidController.h"

PidController::PidController(float p, float i, float d, float maxI) {
    kp = p;
    ki = i;
    kd = d;
    maxIntegral = maxI;
    previousError = 0;
    integral = 0;
}

float PidController::compute(float setpoint, float measuredValue, float dt) {
    float error = setpoint - measuredValue;
    integral += error * dt;
    if (integral > maxIntegral) integral = maxIntegral;
    if (integral < -maxIntegral) integral = -maxIntegral;
    float derivative = (error - previousError) / dt;
    float output = (kp * error) + (ki * integral) + (kd * derivative);
    
    previousError = error;
    
    return output;
}

void PidController::reset() {
    previousError = 0;
    integral = 0;
}