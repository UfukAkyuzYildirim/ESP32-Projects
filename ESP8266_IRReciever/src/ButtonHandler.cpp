#include "ButtonHandler.h"

ButtonHandler::ButtonHandler(uint8_t pin) {
    _pin = pin;
    _lastState = HIGH; 
    _lastStateChangeTime = 0;
    _pressStartTime = 0;
    _longPressHandled = false;
    _clickCount = 0;
    _lastClickTime = 0;
}

void ButtonHandler::begin() {
    pinMode(_pin, INPUT_PULLUP);
    _lastState = digitalRead(_pin);
}

// Bu fonksiyon sadece durumu gunceller
void ButtonHandler::loop() {
    // isLongPressed ve isTripleClicked icinde mantik yurutecegiz
}

bool ButtonHandler::isLongPressed() {
    bool currentState = digitalRead(_pin);
    unsigned long now = millis();

    // Buton basilma ani yakalama ve debounce
    if (currentState != _lastState && (now - _lastStateChangeTime > 50)) {
        _lastStateChangeTime = now;
        _lastState = currentState;
        
        if (currentState == LOW) { // Basildi
            _pressStartTime = now;
            _longPressHandled = false;
        }
    }

    // Basili tutuluyor mu kontrol et
    if (currentState == LOW && !_longPressHandled) {
        if (now - _pressStartTime >= LONG_PRESS_DURATION) {
            _longPressHandled = true; // Islendi olarak isaretle
            return true; // EVENT: Uzun basma gerceklesti
        }
    }
    
    return false;
}

bool ButtonHandler::isTripleClicked() {
    // isLongPressed icinde state guncelleniyor, burada sadece click sayisini kontrol edecegiz
    // Ancak click sayisi "release" aninda artmali.
    
    // Kod tekrarini onlemek icin state okumasini burada tekrar yapiyoruz ama
    // main loop'ta her ikisi de cagirildigi icin sorun olmaz.
    // Daha temiz olmasi icin internal bir update methodu daha iyi olurdu ama 
    // bu basitlikte karistirmayalim.
    
    // Triple click mantigini buraya ozel kuralim:
    static bool btnReleaseHandled = false;
    bool currentState = digitalRead(_pin);
    unsigned long now = millis();
    
    if (currentState == LOW) {
        btnReleaseHandled = false;
    }
    
    // Buton birakildiginda (RISING EDGE)
    if (currentState == HIGH && !btnReleaseHandled) {
        // Debounce kontrolu
        if (now - _lastStateChangeTime > 50) { 
            btnReleaseHandled = true;
            
            // Tiklama araligi kontrolu
            if (now - _lastClickTime < TRIPLE_CLICK_INTERVAL) {
                _clickCount++;
            } else {
                _clickCount = 1;
            }
            _lastClickTime = now;
            
            if (_clickCount >= TRIPLE_CLICK_COUNT) {
                _clickCount = 0; // Sifirla
                return true; // EVENT: 3 kere tiklandi
            }
        }
    }
    
    return false;
}