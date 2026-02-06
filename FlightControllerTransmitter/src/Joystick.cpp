#include "Joystick.h"

// Fiziksel sınırlar
#define X_MIN_VAL 250   
#define X_MAX_VAL 3800  
#define Y_MIN_VAL 200   
#define Y_MAX_VAL 3700  

// Yardımcı Map Fonksiyonu
int mapSmart(int val, int minDead, int maxDead, int minVal, int maxVal) {
    // 1. ÖLÜ BÖLGE
    if (val >= minDead && val <= maxDead) return 0;
    
    // 2. NEGATİF BÖLGE (Sol / Aşağı)
    if (val < minDead) {
        if (val < minVal) val = minVal;
        return map(val, minVal, minDead, -500, 0);
    } 
    // 3. POZİTİF BÖLGE (Sağ / Yukarı)
    else {
        if (val > maxVal) val = maxVal;
        return map(val, maxDead, maxVal, 0, 500);
    }
}

Joystick::Joystick(int x, int y, int sw) {
    this->pinX = x;
    this->pinY = y;
    this->pinSw = sw;
    
    this->isArmed = false;
    this->lastButtonState = HIGH; 
    this->lastDebounceTime = 0;

    // Varsayılan Deadzone
    this->xMinDead = 1750; this->xMaxDead = 1920;
    this->yMinDead = 1600; this->yMaxDead = 1760;

    // Filtreyi sıfırla
    for(int i=0; i<3; i++) {
        historyX[i] = 0;
        historyY[i] = 0;
    }
    historyIndex = 0;
}

void Joystick::begin() {
    pinMode(pinX, INPUT);
    pinMode(pinY, INPUT);
    pinMode(pinSw, INPUT_PULLUP);
}

// FİLTRE FONKSİYONU
int Joystick::getFilteredRaw(int pin, int* history) {
    int newVal = readRaw(pin);

    // Kaydırma
    history[0] = history[1];
    history[1] = history[2];
    history[2] = newVal;

    // Sıralama (Median Bulma)
    int temp[3] = {history[0], history[1], history[2]};
    if (temp[0] > temp[1]) { int t=temp[0]; temp[0]=temp[1]; temp[1]=t; }
    if (temp[1] > temp[2]) { int t=temp[1]; temp[1]=temp[2]; temp[2]=t; }
    if (temp[0] > temp[1]) { int t=temp[0]; temp[0]=temp[1]; temp[1]=t; }

    return temp[1];
}

void Joystick::calibrate() {
    // 1. ISINMA ve FİLTRE DOLDURMA (KRİTİK BÖLÜM!)
    // Hafızadaki o {0,0,0} değerleri gitsin diye 20 kere boşa okuyoruz.
    // Böylece min/max hesaplarken 0'ı görüp yanılmayacak.
    for(int k=0; k<20; k++) {
         getFilteredRaw(pinX, historyX);
         getFilteredRaw(pinY, historyY);
         delay(10);
    }
    
    Serial.println("--- 10 Saniyelik Kalibrasyon ---");
    
    // Değişkenleri şimdi sıfırlıyoruz (Filtre dolduktan sonra)
    int readMinX = 5000; int readMaxX = 0;
    int readMinY = 5000; int readMaxY = 0;
    int samples = 1000; 

    for(int i=0; i<samples; i++) {
        int valX = getFilteredRaw(pinX, historyX);
        int valY = getFilteredRaw(pinY, historyY);

        if (valX < readMinX) readMinX = valX;
        if (valY < readMinY) readMinY = valY;
        if (valX > readMaxX) readMaxX = valX;
        if (valY > readMaxY) readMaxY = valY;

        if (i % 100 == 0) Serial.printf("Kalibrasyon... %d%%\n", i/10);
        delay(10); 
    }

    int k = 60; 

    this->xMinDead = readMinX - k;
    this->xMaxDead = readMaxX + k;
    this->yMinDead = readMinY - k;
    this->yMaxDead = readMaxY + k;

    Serial.println("--- TAMAMLANDI ---");
    Serial.printf("X: [%d - %d] -> DZ: [%d - %d]\n", readMinX, readMaxX, xMinDead, xMaxDead);
    Serial.printf("Y: [%d - %d] -> DZ: [%d - %d]\n", readMinY, readMaxY, yMinDead, yMaxDead);
}

int Joystick::readRaw(int pin) {
    long toplam = 0;
    for (int i = 0; i < 5; i++) { 
        toplam += analogRead(pin);
        delayMicroseconds(10); 
    }
    return (int)(toplam / 5);
}

int Joystick::getX() {
    int rawFiltered = getFilteredRaw(pinX, historyX);
    return mapSmart(rawFiltered, xMinDead, xMaxDead, X_MIN_VAL, X_MAX_VAL);
}

int Joystick::getY() {
    int rawFiltered = getFilteredRaw(pinY, historyY);
    return mapSmart(rawFiltered, yMinDead, yMaxDead, Y_MIN_VAL, Y_MAX_VAL);
}

bool Joystick::getToggleState() {
    int reading = digitalRead(pinSw);
    if (reading != lastButtonState) {
        lastDebounceTime = millis();
    }
    if ((millis() - lastDebounceTime) > 50) {
        static int currentStableState = HIGH;
        if (reading != currentStableState) {
            currentStableState = reading;
            if (currentStableState == LOW) {
                isArmed = !isArmed;
            }
        }
    }
    lastButtonState = reading;
    return isArmed;
}

void Joystick::printDebug() {
    // Loglama için manuel medyan hesabı (getX çağırmadan)
    int tempX[3] = {historyX[0], historyX[1], historyX[2]};
    if (tempX[0] > tempX[1]) { int t=tempX[0]; tempX[0]=tempX[1]; tempX[1]=t; }
    if (tempX[1] > tempX[2]) { int t=tempX[1]; tempX[1]=tempX[2]; tempX[2]=t; }
    if (tempX[0] > tempX[1]) { int t=tempX[0]; tempX[0]=tempX[1]; tempX[1]=t; }
    int medianX = tempX[1];

    int tempY[3] = {historyY[0], historyY[1], historyY[2]};
    if (tempY[0] > tempY[1]) { int t=tempY[0]; tempY[0]=tempY[1]; tempY[1]=t; }
    if (tempY[1] > tempY[2]) { int t=tempY[1]; tempY[1]=tempY[2]; tempY[2]=t; }
    if (tempY[0] > tempY[1]) { int t=tempY[0]; tempY[0]=tempY[1]; tempY[1]=t; }
    int medianY = tempY[1];

    int outX = mapSmart(medianX, xMinDead, xMaxDead, X_MIN_VAL, X_MAX_VAL);
    int outY = mapSmart(medianY, yMinDead, yMaxDead, Y_MIN_VAL, Y_MAX_VAL);

    Serial.printf("X [Raw: %d -> Out: %d] | Y [Raw: %d -> Out: %d] | ARM: %d\n", 
                  medianX, outX, medianY, outY, isArmed);
}