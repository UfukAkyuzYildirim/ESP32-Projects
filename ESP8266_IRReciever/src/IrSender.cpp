#include "IrSender.h"

IrSender::IrSender(uint16_t sendPin) {
    pin = sendPin;
    irsend = new IRsend(pin);
}

void IrSender::begin() {
    irsend->begin();
    Serial.printf("IR Verici baslatildi (Pin: %d)\n", pin);
}

void IrSender::sendRaw(const uint16_t* rawData, uint16_t len, uint16_t freq) {
    irsend->sendRaw(rawData, len, freq);
}

void IrSender::send(decode_results *results) {
    if (results->value == 0 && results->decode_type != UNKNOWN) {
        Serial.println("Gonderilecek gecerli bir kod yok!");
        return;
    }

    Serial.print("Gonderiliyor... Protokol: ");
    Serial.print(typeToString(results->decode_type));
    Serial.print(" Kod: 0x");
    serialPrintUint64(results->value, HEX);
    Serial.println();

    // Protokol tipine gore gonderme islemi
    switch (results->decode_type) {
        case NEC:
            irsend->sendNEC(results->value, results->bits);
            break;
        case SONY:
            irsend->sendSony(results->value, results->bits);
            break;
        // Samsung kaldirildi (Guncel kutuphanede farkli versiyonlari var)
        case RC5:
            irsend->sendRC5(results->value, results->bits);
            break;
        case RC6:
            irsend->sendRC6(results->value, results->bits);
            break;
        case PANASONIC:
            irsend->sendPanasonic(results->value, results->bits);
            break;
        case LG:
            irsend->sendLG(results->value, results->bits);
            break;
        case JVC:
            irsend->sendJVC(results->value, results->bits);
            break;
        case UNKNOWN:
            Serial.println("Bilinmeyen protokol (RAW). Manuel RAW gonderme kullanilmali.");
            break;
        default:
            Serial.println("Bu protokol icin henuz case eklenmedi!");
            break;
    }
}

// Yeni eklenen overload metod
void IrSender::send(decode_type_t type, uint64_t value, uint16_t bits) {
    decode_results tempResults;
    tempResults.decode_type = type;
    tempResults.value = value;
    tempResults.bits = bits;
    this->send(&tempResults);
}
