#ifndef IR_SENDER_H
#define IR_SENDER_H

#include <Arduino.h>
#include <IRsend.h>
#include <IRutils.h>

class IrSender {
private:
    IRsend* irsend;
    uint16_t pin;

public:
    IrSender(uint16_t sendPin);
    void begin();
    void send(decode_results *results); // Yakalanan sonucu gonder
    void send(decode_type_t type, uint64_t value, uint16_t bits); // Manuel deger gonder
    void sendRaw(const uint16_t* rawData, uint16_t len, uint16_t freq); // Raw veri gonder
};

#endif
