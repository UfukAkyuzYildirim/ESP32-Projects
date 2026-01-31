#include "Antenna.h"

// --- AYNI ADRES ---
const uint8_t address[5] = { 0x12, 0x34, 0x56, 0x78, 0x90 };

Antenna::Antenna(int ce, int csn) {
    this->cePin = ce;
    this->csnPin = csn;
    this->radio = new RF24(cePin, csnPin);
}

bool Antenna::begin() {
    if (!radio->begin()) return false;

    // --- TEMEL AYARLAR ---
    radio->setPALevel(RF24_PA_MIN); 
    radio->setDataRate(RF24_250KBPS); 
    radio->setChannel(108);      
    radio->setAutoAck(false);    
    radio->disableDynamicPayloads(); 
    radio->setPayloadSize(1);     
    
    // Verici tarafinda pipe nosu vermeyiz, o direkt adrese yazar.
    // Ama arkada planda Pipe 0'i kullanir.
    radio->openWritingPipe(address); 
    
    radio->stopListening(); 
    return true;
}
// Digerleri ayni...
bool Antenna::send(byte data) { return radio->write(&data, sizeof(data)); }
void Antenna::showDetails() { radio->printDetails(); }
void Antenna::testConnection() {}