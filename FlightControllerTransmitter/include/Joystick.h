#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <Arduino.h>

class Joystick {
private:
    int pinX;
    int pinY;
    int pinSw;

    int minVal = 0;    
    int maxVal = 4095; 
    int centerVal = 1912; 
    int deadzone = 150;   

    int readRaw(int pin);

public:
    Joystick(int x, int y, int sw);
    void begin();

    int getX();
    int getY();
    bool isPressed();
    
    void printDebug();
};

#endif