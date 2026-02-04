#ifndef IMU_SYSTEM_H
#define IMU_SYSTEM_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

struct DroneAngles {
    float pitch;
    float roll;
    float yaw;
};

class ImuSystem {
private:
    Adafruit_BNO055 bno;
    
public:
    ImuSystem();
    bool begin();
    
    DroneAngles getAngles();
    uint8_t getCalibrationSys();
};

#endif