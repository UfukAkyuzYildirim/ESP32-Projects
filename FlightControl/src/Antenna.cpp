#include "Antenna.h"

// --- GARANTILI HEX ADRES ---
const uint8_t address[5] = { 0x12, 0x34, 0x56, 0x78, 0x90 };

Antenna::Antenna(uint8_t ce, uint8_t csn) : radio(ce, csn) {}

bool Antenna::begin() {
    if (!radio.begin()) return false;

    // --- AYARLAR (Kumanda ile Birebir Ayni) ---
    radio.setPALevel(RF24_PA_MIN); 
    radio.setDataRate(RF24_250KBPS); 
    radio.setChannel(108);       // Kanal 108
    radio.setAutoAck(false);     // Otomatik cevap YOK
    
    radio.disableDynamicPayloads(); // Dinamik paket YOK
    radio.setPayloadSize(1);        // Sabit 1 Byte
    
    // SADECE PIPE 0 (Ana Kapi)
    radio.openReadingPipe(0, address); 
    
    radio.startListening(); 
    return true;
}

bool Antenna::isAvailable() { return radio.available(); }
void Antenna::readData(void* data, size_t size) { radio.read(data, size); }
void Antenna::printDetails() { radio.printDetails(); }