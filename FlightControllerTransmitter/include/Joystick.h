#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <Arduino.h>

class Joystick {
private:
    int pinX;
    int pinY;
    int pinSw;

    bool isArmed;           
    int lastButtonState;    
    unsigned long lastDebounceTime; 

    // Kalibrasyon Sınırları
    int xMinDead, xMaxDead;
    int yMinDead, yMaxDead;

    // --- YENİ: FİLTRE İÇİN HAFIZA ---
    int historyX[3]; // X için son 3 değer
    int historyY[3]; // Y için son 3 değer
    int historyIndex; // Hangi sıradayız?

    int readRaw(int pin);
    int getFilteredRaw(int pin, int* history); // Yeni Filtre Fonksiyonu

public:
    Joystick(int x, int y, int sw);
    void begin();
    void calibrate();

    int getX();
    int getY();
    bool getToggleState(); 
    void printDebug();
};

#endif