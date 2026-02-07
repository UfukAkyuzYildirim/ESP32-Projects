#ifndef DSHOT_FLIGHT_CONTROLLER_H
#define DSHOT_FLIGHT_CONTROLLER_H

#include <Arduino.h>
#include "DShotMotorSystem.h"
#include "RadioSystem.h"
#include "ImuSystem.h"
#include "PidController.h" 

class DShotFlightController {
public:
    DShotFlightController(DShotMotorSystem &motors, RadioSystem &radio, ImuSystem &imu);
    
    bool begin();
    void loopStep();
    
    void calibrateIMU(); // Calibrate IMU

private:
    DShotMotorSystem &motors;
    RadioSystem &radio;
    ImuSystem &imu;

    PidController pidPitch;
    PidController pidRoll;
    PidController pidYaw;

    float currentThrottle;
    unsigned long lastTime;
    unsigned long lastLogTime;
    
    float pitchOffset; // Pitch offset
    float rollOffset;  // Roll offset

    void mixMotors(float throttle, float pitchPid, float rollPid, float yawPid);
};

#endif