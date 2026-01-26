#include "ButtonHandler.h"

ButtonHandler::ButtonHandler(uint8_t pin) {
    _pin = pin;
    _lastState = HIGH; 
    _lastStateChangeTime = 0;
    _pressStartTime = 0;
    _lastHeldMs = 0;
    _releaseEventPending = false;
    _isPressed = false;
    _longPressHandled = false;
    _clickCount = 0;
    _lastClickTime = 0;
}

void ButtonHandler::begin() {
    pinMode(_pin, INPUT_PULLUP);
    _lastState = digitalRead(_pin);
    _isPressed = (_lastState == LOW);
    if (_isPressed) {
        _pressStartTime = millis();
    }
}

// Bu fonksiyon sadece durumu gunceller
void ButtonHandler::loop() {
    bool currentState = digitalRead(_pin);
    unsigned long now = millis();

    // Debounce + edge yakalama
    if (currentState != _lastState && (now - _lastStateChangeTime > 50)) {
        _lastStateChangeTime = now;
        bool prevState = _lastState;
        _lastState = currentState;

        if (prevState == HIGH && currentState == LOW) {
            // Press
            _pressStartTime = now;
            _longPressHandled = false;
            _isPressed = true;
        } else if (prevState == LOW && currentState == HIGH) {
            // Release
            _lastHeldMs = now - _pressStartTime;
            _releaseEventPending = true;
            _isPressed = false;

            // Click sayisini sadece "kisa basma" icin sayalim
            // (Uzun basmalarin triple-click'i yanlis tetiklemesini engeller)
            if (_lastHeldMs < LEARN_PRESS_DURATION) {
                if (now - _lastClickTime < TRIPLE_CLICK_INTERVAL) {
                    _clickCount++;
                } else {
                    _clickCount = 1;
                }
                _lastClickTime = now;
            } else {
                _clickCount = 0;
            }
        }
    }
}

bool ButtonHandler::isPressed() const {
    return _isPressed;
}

unsigned long ButtonHandler::getHeldMs() const {
    if (_isPressed) {
        return millis() - _pressStartTime;
    }
    return _lastHeldMs;
}

bool ButtonHandler::getReleaseEvent(unsigned long& heldMs) {
    if (_releaseEventPending) {
        _releaseEventPending = false;
        heldMs = _lastHeldMs;
        return true;
    }
    return false;
}

bool ButtonHandler::isLongPressed() {
    // Geriye donuk API: 5 sn basili tutulunca bir kez true
    // Not: Yeni tasarimda esas olan getReleaseEvent() ile sureye gore karar vermek.
    bool currentState = digitalRead(_pin);
    unsigned long now = millis();

    // loop() cagrilmadiysa da calissin diye minimal state guncelle
    if (currentState != _lastState && (now - _lastStateChangeTime > 50)) {
        _lastStateChangeTime = now;
        _lastState = currentState;
        if (currentState == LOW) {
            _pressStartTime = now;
            _longPressHandled = false;
        }
    }

    if (currentState == LOW && !_longPressHandled) {
        if (now - _pressStartTime >= AP_PRESS_DURATION) {
            _longPressHandled = true;
            return true;
        }
    }
    return false;
}

bool ButtonHandler::isTripleClicked() {
    if (_clickCount >= TRIPLE_CLICK_COUNT) {
        _clickCount = 0;
        return true;
    }
    return false;
}