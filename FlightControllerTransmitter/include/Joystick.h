#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <Arduino.h>

class Joystick {
private:
    int pinX;
    int pinY;
    int pinSw;
    
    int readRaw(int pin);

public:
    Joystick(int x, int y, int sw);
    void begin();

    int getX();
    int getY();
    bool isPressed();
};

#endif