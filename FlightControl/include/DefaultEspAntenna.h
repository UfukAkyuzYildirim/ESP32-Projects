#ifndef DEFAULT_ESP_ANTENNA_H
#define DEFAULT_ESP_ANTENNA_H

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>

// Basit ESP-NOW anten sinifi (ALICI veya GONDERICI icin)
// nRF24 kodunu bozmadan yaninda calismasi icin yeni bir sinif.
class DefaultEspAntenna {
public:
    // isSender=true ise gonderici modunda calisir.
    // peerMac: karsi tarafin MAC adresi (6 byte)
    DefaultEspAntenna(const uint8_t peerMac[6], bool isSender);

    bool begin();

    // Gonderici fonksiyonu (isSender=true iken kullanilir)
    bool sendByte(uint8_t data);

    // Alici fonksiyonlari (isSender=false iken kullanilir)
    bool isAvailable() const;
    uint8_t readByte();

private:
    bool sender;
    uint8_t peer[6];

    // Alici icin son paket
    static volatile bool hasData;
    static volatile uint8_t lastByte;
    static DefaultEspAntenna* instance;

    static void onReceive(const uint8_t* mac, const uint8_t* data, int len);
};

#endif
