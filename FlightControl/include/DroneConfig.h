#ifndef DRONE_CONFIG_H
#define DRONE_CONFIG_H

#include <Arduino.h>

#define PIN_FL 4
#define PIN_FR 17
#define PIN_RL 13

#define PIN_RR 21
#define MIN_PULSE 1000
#define MAX_PULSE 2000

#define MIN_START_PWM 1036 
#define IDLE_PWM      1036

// Onboard LED pin (ESP32-S3 DevKitC yeşil genelde GPIO48). Değiştirilecekse burayı güncelle.
#ifndef LED_PIN_ONBOARD
#define LED_PIN_ONBOARD 48
#endif

// Onboard RGB (WS2812) pin; birçok S3 kitinde 48 kullanılır.
#ifndef LED_RGB_PIN
#define LED_RGB_PIN 48
#endif

#define CONNECTION_TIMEOUT 1000 
#define MAX_TILT_ANGLE 45.0 

#endif