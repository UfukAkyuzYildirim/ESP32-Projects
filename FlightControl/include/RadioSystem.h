#ifndef RADIO_SYSTEM_H
#define RADIO_SYSTEM_H

#include <Arduino.h>
#include "DroneConfig.h"
#include <esp_now.h>
#include <WiFi.h>

typedef struct DronePacket {
    int x;
    int y;
    int sw;
} DronePacket;

class RadioSystem {
private:
    DronePacket currentData;
    unsigned long lastRecvTime;
    static void onDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);
    static RadioSystem* instance;

public:
    RadioSystem();
    bool begin();
    bool isConnectionAlive();
    
    int getThrottle(); 
    int getRoll();     
    int getSwitch();   
    int getRawY(); 
};

#endif