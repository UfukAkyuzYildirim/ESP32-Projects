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
    
    // YENI FONKSIYON: Sifir noktasina gore donus acisini verir (-180..+180)
    float getRelativeYaw(); // Getter for easy logging

    // Motor Degerlerini Log icin Disari Acma
    float motorFL, motorFR, motorRL, motorRR;

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
    float yawOffset;   // YENI: Yaw offset (0 noktasi)
    float targetHeading; // YENI: Hedef Yaw (Heading Hold)

    // YENI: Aci farkini en kisa yoldan hesaplar (-180..180)
    float getErrorShortestPath(float target, float current);

    void mixMotors(float throttle, float pitchPid, float rollPid, float yawPid);
};

#endif