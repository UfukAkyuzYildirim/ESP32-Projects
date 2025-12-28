#include "IrSender.h"

IrSender::IrSender(uint16_t sendPin) {
    pin = sendPin;
    irsend = new IRsend(pin);
}

void IrSender::begin() {
    irsend->begin();
    Serial.printf("IR Verici baslatildi (Pin: %d)\n", pin);
}

void IrSender::send(decode_results *results) {
    if (results->value == 0) {
        Serial.println("Gonderilecek gecerli bir kod yok!");
        return;
    }

    Serial.print("Gonderiliyor... Protokol: ");
    Serial.print(typeToString(results->decode_type));
    Serial.print(" Kod: 0x");
    serialPrintUint64(results->value, HEX);
    Serial.println();

    // Protokol tipine gore gonderme islemi
    // Kutuphane her protokol icin ayri fonksiyon kullaniyor, 
    // en yaygin olanlari buraya ekledim.
    switch (results->decode_type) {
        case NEC:
            irsend->sendNEC(results->value, results->bits);
            break;
        case SONY:
            irsend->sendSony(results->value, results->bits);
            break;
        /* case SAMSUNG:
            irsend->sendSamsung(results->value, results->bits);
            break; */
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
            Serial.println("Bilinmeyen protokol (RAW) algilandi. Ham veri gonderiliyor...");
            {
                // Raw veriyi gondermek icin uygun formata cevir (Mikrosaniye cinsinden)
                // resultToRawArray fonksiyonu dinamik bellek ayirir, is bitince silmeliyiz.
                uint16_t *raw_array = resultToRawArray(results);
                // rawlen eleman sayisidir, sendRaw frekans olarak genellikle 38kHz kullanir.
                results->rawlen = results->rawlen - 1; // Genelde ilk eleman atlanir
                irsend->sendRaw(raw_array, results->rawlen, 38); 
                delete [] raw_array; // Bellek sizintisini onle
                Serial.println("RAW gonderme tamamlandi.");
            }
            break;
        default:
            Serial.println("Bu protokol icin henuz case eklenmedi, kutuphaneden eklenebilir.");
            // Diger protokoller icin (Coolix, Whynter vs) buraya ekleme yapilabilir.
            // irsend->sendCoolix(...) gibi.
            break;
    }
}
