#ifndef DEFAULT_ESP_ANTENNA_H
#define DEFAULT_ESP_ANTENNA_H

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>

class DefaultEspAntenna {
public:
    DefaultEspAntenna(const uint8_t peerMac[6], bool isSender);

    bool begin();
    bool sendByte(uint8_t data);
    bool isAvailable() const;
    uint8_t readByte();

private:
    bool sender;
    uint8_t peer[6];

    static volatile bool hasData;
    static volatile uint8_t lastByte;
    static DefaultEspAntenna* instance;

    static void onReceive(const uint8_t* mac, const uint8_t* data, int len);
};

#endif
