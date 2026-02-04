#include "Joystick.h"

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
    for (int i = 0; i < 10; i++) {
        toplam += analogRead(pin);
        delayMicroseconds(100); 
    }
    return (int)(toplam / 10);
}

int Joystick::getX() {
    int raw = readRaw(pinX);
    if (abs(raw - centerVal) < deadzone) return 0;
    int mappedValue = map(raw, minVal, maxVal, -1000, 1000);
    return constrain(mappedValue, -1000, 1000);
}

int Joystick::getY() {
    int raw = readRaw(pinY);
    if (abs(raw - centerVal) < deadzone) return 0;
    int mappedValue = map(raw, minVal, maxVal, -1000, 1000);
    return constrain(mappedValue, -1000, 1000);
}

bool Joystick::isPressed() {
    return digitalRead(pinSw) == LOW; 
}

void Joystick::printDebug() {
    Serial.print("X: "); Serial.print(getX());
    Serial.print(" | Y: "); Serial.print(getY());
    Serial.println();
}