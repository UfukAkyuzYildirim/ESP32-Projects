#ifndef IR_READER_H
#define IR_READER_H

#include <Arduino.h>
#include <IRrecv.h>
#include <IRutils.h>

class IrReader {
private:
    IRrecv* irrecv;
    decode_results results;
    uint16_t pin;
    bool hasNewCode;

public:
    IrReader(uint16_t recvPin);
    void begin();
    bool loop(); // Dongude surekli cagrilacak
    decode_results getResults(); // Son okunan veriyi dondurur
    void resume(); // Okumaya devam et
};

#endif
