#ifndef ANTENNA_H
#define ANTENNA_H

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

class Antenna {
private:
    RF24 radio;

public:
    Antenna(uint8_t ce, uint8_t csn);
    bool begin();
    bool isAvailable();
    void readData(void* data, size_t size);
    void printDetails();
};

#endif