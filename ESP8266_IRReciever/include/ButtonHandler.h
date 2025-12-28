#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <Arduino.h>
#include "Config.h"

class ButtonHandler {
public:
    ButtonHandler(uint8_t pin);
    void begin();
    void loop(); // Loop icinde surekli cagirilacak

    bool isLongPressed();     // Uzun basma tetiklendi mi? (Bir kez true doner)
    bool isTripleClicked();   // 3 kez tiklama tetiklendi mi? (Bir kez true doner)

private:
    uint8_t _pin;
    bool _lastState;
    unsigned long _lastStateChangeTime;
    unsigned long _pressStartTime;
    
    // Long Press
    bool _longPressHandled;

    // Multi Click
    uint8_t _clickCount;
    unsigned long _lastClickTime;
    bool _tripleClickTriggered;
};

#endif
