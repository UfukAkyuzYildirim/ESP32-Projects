#include "Logger.h"

float Logger::lastFloatVal = -999.0;
int Logger::lastIntVal = -999;
String Logger::lastStringVal = "";
String Logger::lastLabel = "";

void Logger::begin() {
    // WebSerial zaten main içinde başladığı için burada ekstra işleme gerek yok
    // İleride log dosyası tutmak istersek burasi kullanilacak.
}

void Logger::log(const char* message) {
    WebSerial.println(message);
    Serial.println(message);
}

// Float değerler için (Örn: Sensör mesafesi)
void Logger::logIfChanged(const char* label, float currentVal, float threshold) {
    if (abs(currentVal - lastFloatVal) > threshold || String(label) != lastLabel) {
        WebSerial.printf("%s: %.2f\n", label, currentVal);
        Serial.printf("%s: %.2f\n", label, currentVal);
        lastFloatVal = currentVal;
        lastLabel = label;
    }
}

// Integer değerler için (Örn: Kumanda sinyali 0-100)
void Logger::logIfChanged(const char* label, int currentVal) {
    if (currentVal != lastIntVal || String(label) != lastLabel) {
        WebSerial.printf("%s: %d\n", label, currentVal);
        Serial.printf("%s: %d\n", label, currentVal);
        lastIntVal = currentVal;
        lastLabel = label;
    }
}

// String/Durum mesajları için (Örn: "ARMED", "IDLE")
void Logger::logIfChanged(const char* label, const char* currentVal) {
    if (String(currentVal) != lastStringVal || String(label) != lastLabel) {
        WebSerial.printf("%s: %s\n", label, currentVal);
        Serial.printf("%s: %s\n", label, currentVal);
        lastStringVal = currentVal;
        lastLabel = label;
    }
}