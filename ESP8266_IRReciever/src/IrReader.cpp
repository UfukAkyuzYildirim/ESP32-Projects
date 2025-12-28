#include "IrReader.h"

IrReader::IrReader(uint16_t recvPin) {
    pin = recvPin;
    irrecv = new IRrecv(pin);
    hasNewCode = false;
}

void IrReader::begin() {
    irrecv->enableIRIn(); // Aliciyi baslat
    Serial.printf("IR Alici baslatildi (Pin: %d)\n", pin);
}

bool IrReader::loop() {
    if (irrecv->decode(&results)) {
        // Yeni bir kod yakalandi
        hasNewCode = true;
        
        // Ekrana detaylari basalim
        Serial.println("----------------------------------------");
        Serial.print("Protokol: ");
        Serial.print(typeToString(results.decode_type));
        Serial.print(" | Kod: 0x");
        serialPrintUint64(results.value, HEX);
        Serial.print(" | Bit: ");
        Serial.println(results.bits);
        Serial.println("----------------------------------------");
        
        return true;
    }
    return false;
}

decode_results IrReader::getResults() {
    return results;
}

void IrReader::resume() {
    irrecv->resume(); // Bir sonraki sinyali dinlemeye hazir ol
    hasNewCode = false;
}

