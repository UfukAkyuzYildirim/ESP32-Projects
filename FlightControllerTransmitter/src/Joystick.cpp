#include "Joystick.h"

// ADC border
#define MIN_RAW 0
#define MAX_RAW 4095

// Output border (-500 / +500)
#define OUT_MIN -500
#define OUT_MAX 500

int mapSmart(int val, int minDead, int maxDead) {
    if (val >= minDead && val <= maxDead) return 0; // deadzone
    
    if (val < minDead) {
        return map(val, MIN_RAW, minDead, OUT_MIN, 0);
    } else {
        return map(val, maxDead, MAX_RAW, 0, OUT_MAX);
    }
}

Joystick::Joystick(int lx, int ly, int rx, int ry, int swL, int swR) {
    this->pinLX = lx; this->pinLY = ly;
    this->pinRX = rx; this->pinRY = ry;
    this->pinSwL = swL; this->pinSwR = swR;
    
    this->isArmed = false;
    this->lastButtonState = HIGH; 
    this->lastDebounceTime = 0;

    // Varsayılan Deadzone
    lxMinDead=1800; lxMaxDead=2200;
    lyMinDead=1800; lyMaxDead=2200;
    rxMinDead=1800; rxMaxDead=2200;
    ryMinDead=1800; ryMaxDead=2200;

    for(int i=0; i<3; i++) {
        historyLX[i]=0; historyLY[i]=0;
        historyRX[i]=0; historyRY[i]=0;
    }
}

void Joystick::begin() {
    pinMode(pinLX, INPUT); pinMode(pinLY, INPUT);
    pinMode(pinRX, INPUT); pinMode(pinRY, INPUT);
    pinMode(pinSwL, INPUT_PULLUP);
    pinMode(pinSwR, INPUT_PULLUP);
}

int Joystick::readRaw(int pin) {
    long t = 0;
    for (int i = 0; i < 5; i++) { t += analogRead(pin); delayMicroseconds(10); }
    return (int)(t / 5);
}

int Joystick::getFilteredRaw(int pin, int* history) {
    int val = readRaw(pin);
    history[0] = history[1]; history[1] = history[2]; history[2] = val;
    int tmp[3] = {history[0], history[1], history[2]};
    if (tmp[0]>tmp[1]) { int t=tmp[0]; tmp[0]=tmp[1]; tmp[1]=t; }
    if (tmp[1]>tmp[2]) { int t=tmp[1]; tmp[1]=tmp[2]; tmp[2]=t; }
    if (tmp[0]>tmp[1]) { int t=tmp[0]; tmp[0]=tmp[1]; tmp[1]=t; }
    return tmp[1];
}

void Joystick::calibrate() {
    for(int k=0; k<20; k++) {
         getFilteredRaw(pinLX, historyLX); getFilteredRaw(pinLY, historyLY);
         getFilteredRaw(pinRX, historyRX); getFilteredRaw(pinRY, historyRY);
         delay(10);
    }
    
    Serial.println("--- KALIBRASYON (3 Sn) ---");
    int minLX=5000, maxLX=0; int minLY=5000, maxLY=0;
    int minRX=5000, maxRX=0; int minRY=5000, maxRY=0;

    for(int i=0; i<300; i++) {
        int vLX = getFilteredRaw(pinLX, historyLX);
        int vLY = getFilteredRaw(pinLY, historyLY);
        int vRX = getFilteredRaw(pinRX, historyRX);
        int vRY = getFilteredRaw(pinRY, historyRY);

        if(vLX<minLX) minLX=vLX; if(vLX>maxLX) maxLX=vLX;
        if(vLY<minLY) minLY=vLY; if(vLY>maxLY) maxLY=vLY;
        if(vRX<minRX) minRX=vRX; if(vRX>maxRX) maxRX=vRX;
        if(vRY<minRY) minRY=vRY; if(vRY>maxRY) maxRY=vRY;
        if(i%30==0) Serial.print(".");
        delay(10); 
    }
    Serial.println("\n--- BITTI ---");

    int k = 60; 
    lxMinDead = minLX-k; lxMaxDead = maxLX+k;
    lyMinDead = minLY-k; lyMaxDead = maxLY+k;
    rxMinDead = minRX-k; rxMaxDead = maxRX+k;
    ryMinDead = minRY-k; ryMaxDead = maxRY+k;
}

int Joystick::getLX() { return mapSmart(getFilteredRaw(pinLX, historyLX), lxMinDead, lxMaxDead); }
int Joystick::getLY() { return mapSmart(getFilteredRaw(pinLY, historyLY), lyMinDead, lyMaxDead); }
int Joystick::getRX() { return mapSmart(getFilteredRaw(pinRX, historyRX), rxMinDead, rxMaxDead); }
int Joystick::getRY() { return mapSmart(getFilteredRaw(pinRY, historyRY), ryMinDead, ryMaxDead); }

bool Joystick::getToggleState() {
    int sL = digitalRead(pinSwL);
    int sR = digitalRead(pinSwR);
    int state = (sL == LOW && sR == LOW) ? LOW : HIGH;

    if (state != lastButtonState) lastDebounceTime = millis();
    if ((millis() - lastDebounceTime) > 50) {
        static int stable = HIGH;
        if (state != stable) {
            stable = state;
            if (stable == LOW) isArmed = !isArmed;
        }
    }
    lastButtonState = state;
    return isArmed;
}

void Joystick::printDebug() {
    Serial.printf("SOL[X:%d Y:%d] | SAG[X:%d Y:%d] | ARM: %d\n", 
                  getLX(), getLY(), getRX(), getRY(), isArmed);
}