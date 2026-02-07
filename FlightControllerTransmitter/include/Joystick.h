#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <Arduino.h>

class Joystick {
private:
    int pinLX, pinLY; 
    int pinRX, pinRY; 
    int pinSwL, pinSwR;

    bool isArmed;           
    int lastButtonState;    
    unsigned long lastDebounceTime; 

    // Deadzone
    int lxMinDead, lxMaxDead;
    int lyMinDead, lyMaxDead;
    int rxMinDead, rxMaxDead;
    int ryMinDead, ryMaxDead;

    // MedianFilter history
    int historyLX[3]; int historyLY[3];
    int historyRX[3]; int historyRY[3];

    int readRaw(int pin);
    int getFilteredRaw(int pin, int* history); 

public:
    Joystick(int lx, int ly, int rx, int ry, int swL, int swR);
    
    void begin();
    void calibrate();

    int getLX(); int getLY(); 
    int getRX(); int getRY();
    
    bool getToggleState(); 
    void printDebug();
};

#endif