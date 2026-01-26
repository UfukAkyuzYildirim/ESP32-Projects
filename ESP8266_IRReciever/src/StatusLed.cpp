#include "StatusLed.h"

StatusLed::StatusLed(uint8_t pin, bool activeLow) {
    _pin = pin;
    _ledState = false;
    _activeLow = activeLow;
    _lastToggleTime = 0;
    _onTime = 500;
    _offTime = 500;
}

void StatusLed::writePin(bool logicalOn) {
    // logicalOn=true => LED yanik
    uint8_t level = logicalOn ? HIGH : LOW;
    if (_activeLow) {
        level = logicalOn ? LOW : HIGH;
    }
    digitalWrite(_pin, level);
}

void StatusLed::begin() {
    pinMode(_pin, OUTPUT);
    writePin(false);
}

void StatusLed::on() {
    writePin(true);
    _ledState = true;
}

void StatusLed::off() {
    writePin(false);
    _ledState = false;
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
        writePin(_ledState);
    }
}
