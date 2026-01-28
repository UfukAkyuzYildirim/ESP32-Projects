#ifndef ANTENNA_H
#define ANTENNA_H

#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>

class Antenna {
private:
    RF24* radio; // Managed dynamically to control initialization timing
    int cePin;
    int csnPin;

public:
    Antenna(int ce, int csn);
    bool begin();
    void showDetails();
};

#endif