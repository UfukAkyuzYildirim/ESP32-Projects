#include "Antenna.h"

Antenna::Antenna(int ce, int csn) {
    this->cePin = ce;
    this->csnPin = csn;
    this->radio = new RF24(cePin, csnPin);
}

bool Antenna::begin() {
    if (!radio->begin()) {
        return false;
    }
    radio->setPALevel(RF24_PA_LOW);
    return true;
}

void Antenna::showDetails() {
    radio->printDetails();
}