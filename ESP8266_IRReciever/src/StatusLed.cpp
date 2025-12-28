#include "StatusLed.h"

StatusLed::StatusLed(uint8_t pin) {
    _pin = pin;
    _ledState = LOW;
    _lastToggleTime = 0;
    _onTime = 500;
    _offTime = 500;
}

void StatusLed::begin() {
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
}

void StatusLed::on() {
    digitalWrite(_pin, HIGH);
    _ledState = HIGH;
}

void StatusLed::off() {
    digitalWrite(_pin, LOW);
    _ledState = LOW;
}

void StatusLed::setBlinkPattern(unsigned long onTime, unsigned long offTime) {
    _onTime = onTime;
    _offTime = offTime;
}

void StatusLed::blink(unsigned long onTime, unsigned long offTime) {
    // Desen degistiyse guncelle
    if (onTime != _onTime || offTime != _offTime) {
        setBlinkPattern(onTime, offTime);
    }
    
    unsigned long now = millis();
    unsigned long interval = _ledState ? _onTime : _offTime;

    if (now - _lastToggleTime >= interval) {
        _lastToggleTime = now;
        _ledState = !_ledState;
        digitalWrite(_pin, _ledState);
    }
}
