#ifndef PWM_FLIGHT_CONTROLLER_H
#define PWM_FLIGHT_CONTROLLER_H

#include <Arduino.h>
#include "MotorSystem.h"
#include "RadioSystem.h"

// Legacy PWM throttle controller for analog ESCs.
class PwmFlightController {
public:
    PwmFlightController(MotorSystem &motors, RadioSystem &radio);
    bool begin();
    void loopStep();

private:
    MotorSystem &motors;
    RadioSystem &radio;
    float currentPwm;
    unsigned long lastTime;
    unsigned long lastLogTime;
};

#endif
