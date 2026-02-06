#include "ImuSystem.h"

ImuSystem::ImuSystem() : bno(55, 0x28) {}

bool ImuSystem::begin() {
    Wire.begin(1, 2); 

    if (!bno.begin()) {
        return false;
    }
    
    delay(100);
    bno.setExtCrystalUse(true);
    return true;
}

DroneAngles ImuSystem::getAngles() {
    sensors_event_t event; 
    bno.getEvent(&event);
    
    DroneAngles angles;
    angles.pitch = event.orientation.y; 
    angles.roll  = -event.orientation.z; 
    angles.yaw   = event.orientation.x;

    return angles;
}

uint8_t ImuSystem::getCalibrationSys() {
    uint8_t sys, gyro, accel, mag;
    bno.getCalibration(&sys, &gyro, &accel, &mag);
    return sys; 
}

void ImuSystem::getCalibration(uint8_t &sys, uint8_t &gyro, uint8_t &accel, uint8_t &mag) {
    bno.getCalibration(&sys, &gyro, &accel, &mag);
}