#ifndef RADIO_SYSTEM_H
#define RADIO_SYSTEM_H

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include "DroneConfig.h"

// Kumanda paketi (5 alan)
typedef struct DronePacket {
    int lx; // Sol X  (-500..500)
    int ly; // Sol Y  (-500..500)
    int rx; // Sağ X  (-500..500)
    int ry; // Sağ Y  (-500..500)
    int sw; // Arm toggle (0/1)
} DronePacket;

class RadioSystem {
private:
    DronePacket currentData;
    unsigned long lastRecvTime;
    
    // Static callback fonksiyonu
    static void onDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);
    static RadioSystem* instance; 

public:
    RadioSystem();
    bool begin();
    bool isConnectionAlive(); 
    
    int getLX();
    int getLY();
    int getRX();
    int getRY();

    int getX(); // backward compat: roll uses RX
    int getY(); // backward compat: lift uses LY

    int getLiftSpeed(); 
    int getRollRate(); 
    bool isSwitchOn(); 
    
    // --- ESKİ FONKSİYONLAR
    int getThrottle(); 
    int getRoll();     
    int getSwitch();   
    int getRawY(); 
};

#endif