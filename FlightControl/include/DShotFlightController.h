#ifndef DSHOT_FLIGHT_CONTROLLER_H
#define DSHOT_FLIGHT_CONTROLLER_H

#include <Arduino.h>
#include "DShotMotorSystem.h"
#include "RadioSystem.h"

class DShotFlightController {
public:
    DShotFlightController(DShotMotorSystem &motors, RadioSystem &radio);
    bool begin();
    void loopStep();

private:
    DShotMotorSystem &motors;
    RadioSystem &radio;
    float currentUs;
    unsigned long lastTime;
    unsigned long lastLogTime;
};

#endif
