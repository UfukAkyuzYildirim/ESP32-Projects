#include "Antenna.h"

const uint8_t address[5] = { 0x12, 0x34, 0x56, 0x78, 0x90 };

Antenna::Antenna(uint8_t ce, uint8_t csn) : radio(ce, csn) {}

bool Antenna::begin() {
    if (!radio.begin()) return false;

    radio.setPALevel(RF24_PA_MIN); 
    radio.setDataRate(RF24_250KBPS); 
    radio.setChannel(108);
    radio.setAutoAck(false);

    radio.disableDynamicPayloads();
    radio.setPayloadSize(1);

    radio.openReadingPipe(0, address); 
    
    radio.startListening(); 
    return true;
}

bool Antenna::isAvailable() { return radio.available(); }
void Antenna::readData(void* data, size_t size) { radio.read(data, size); }
void Antenna::printDetails() { radio.printDetails(); }