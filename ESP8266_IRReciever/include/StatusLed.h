#ifndef STATUS_LED_H
#define STATUS_LED_H

#include <Arduino.h>

class StatusLed {
public:
    StatusLed(uint8_t pin);
    void begin();
    void on();
    void off();
    void blink(unsigned long onTime, unsigned long offTime); // Loop icinde surekli cagirilacak
    void setBlinkPattern(unsigned long onTime, unsigned long offTime);

private:
    uint8_t _pin;
    unsigned long _lastToggleTime;
    bool _ledState;
    unsigned long _onTime;
    unsigned long _offTime;
};

#endif
