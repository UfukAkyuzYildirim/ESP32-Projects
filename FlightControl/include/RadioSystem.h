#ifndef RADIO_SYSTEM_H
#define RADIO_SYSTEM_H

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include "DroneConfig.h"

// --- YENİ PAKET YAPISI (Kumanda ile Birebir Aynı) ---
typedef struct DronePacket {
    int x;  // Roll: -500 (Sol) ... 0 ... +500 (Sağ)
    int y;  // Lift: -500 (İn) ... 0 (Sabit) ... +500 (Çık)
    int sw; // Switch: 0 veya 1
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