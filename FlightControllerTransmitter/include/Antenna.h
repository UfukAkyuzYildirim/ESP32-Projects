#ifndef ANTENNA_H
#define ANTENNA_H

#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>

class Antenna {
private:
    RF24* radio;
    int cePin;
    int csnPin;

public:
    Antenna(int ce, int csn);
    bool begin();
    bool send(byte data);
    void showDetails();
    void testConnection();
};

#endif