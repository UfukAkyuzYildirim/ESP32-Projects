#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <Arduino.h>

class Joystick {
private:
    int pinX;
    int pinY;
    int pinSw;

    // Calibration settings for raw analog range
    int minVal = 0;    // Lowest observed ADC reading
    int maxVal = 4095; // Highest observed ADC reading
    int centerVal = 1912; // Neutral joystick position
    int deadzone = 150;   // Ignore noise around the center

    // Internal helper that averages multiple samples
    int readRaw(int pin);

public:
    Joystick(int x, int y, int sw);
    void begin();

    // Returns filtered axes in the -1000..+1000 range
    int getX();
    int getY();
    bool isPressed();
    
    void printDebug();
};

#endif