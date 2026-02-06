#include "Joystick.h"

// --- 1. DEADZONE AYARLARI (Merkezleme) ---
// Senin Loglara Gore: X~1815, Y~1690'da duruyor.
// Biz buna +/- 60 guvenlik payi verelim.
#define X_MIN_DEAD 1750
#define X_MAX_DEAD 1880

#define Y_MIN_DEAD 1620
#define Y_MAX_DEAD 1760

// --- 2. FIZIKSEL MENZIL AYARLARI (Kalibrasyon) ---
// Senin joystick 0'a veya 4095'e tam degmiyor.
// O yuzden sinirlari biraz daraltalim ki tam guc alabilelim.

// X: En sol ~190, En sag ~3975
#define X_MIN_VAL 250   // 250'nin alti artik -500 sayilir
#define X_MAX_VAL 3800  // 3800'un ustu artik +500 sayilir

// Y: En asagi ~15, En yukari ~3848
#define Y_MIN_VAL 200   // 200'un alti tam inis (-500)
#define Y_MAX_VAL 3700  // 3700'un ustu tam kalkis (+500)

Joystick::Joystick(int x, int y, int sw) {
    this->pinX = x;
    this->pinY = y;
    this->pinSw = sw;
}

void Joystick::begin() {
    pinMode(pinX, INPUT);
    pinMode(pinY, INPUT);
    pinMode(pinSw, INPUT_PULLUP);
}

int Joystick::readRaw(int pin) {
    long toplam = 0;
    // Gurultu onleyici ortalama
    for (int i = 0; i < 10; i++) {
        toplam += analogRead(pin);
        delayMicroseconds(50); 
    }
    return (int)(toplam / 10);
}

// GELISTIRILMIS AKILLI HARITALAMA
int smartMap(int val, int minDead, int maxDead, int minVal, int maxVal) {
    // 1. MERKEZ KONTROLU
    if (val >= minDead && val <= maxDead) return 0;
    
    // 2. NEGATIF YON (Sola veya Asagi)
    if (val < minDead) {
        // Fiziksel siniri asmissa kirpalim
        if (val < minVal) val = minVal;
        
        // minVal ile minDead arasini -> -500 ile 0 yap
        return map(val, minVal, minDead, -500, 0);
    } 
    // 3. POZITIF YON (Saga veya Yukari)
    else {
        // Fiziksel siniri asmissa kirpalim
        if (val > maxVal) val = maxVal;
        
        // maxDead ile maxVal arasini -> 0 ile +500 yap
        return map(val, maxDead, maxVal, 0, 500);
    }
}

int Joystick::getX() {
    return smartMap(readRaw(pinX), X_MIN_DEAD, X_MAX_DEAD, X_MIN_VAL, X_MAX_VAL);
}

int Joystick::getY() {
    return smartMap(readRaw(pinY), Y_MIN_DEAD, Y_MAX_DEAD, Y_MIN_VAL, Y_MAX_VAL);
}

bool Joystick::isPressed() {
    return digitalRead(pinSw) == LOW;
}