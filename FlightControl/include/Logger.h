#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include <WebSerial.h>

class Logger {
public:
    static void begin();
    
    static void log(const char* message);

    static void logIfChanged(const char* label, float currentVal, float threshold = 0.1);
    static void logIfChanged(const char* label, int currentVal);
    static void logIfChanged(const char* label, const char* currentVal);

private:
    static float lastFloatVal;
    static int lastIntVal;
    static String lastStringVal;
    static String lastLabel;
};

#endif