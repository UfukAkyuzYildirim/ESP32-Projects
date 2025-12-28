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
};

#endif
