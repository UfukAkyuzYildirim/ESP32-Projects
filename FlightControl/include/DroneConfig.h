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
#define IDLE_PWM      1046

#define CONNECTION_TIMEOUT 1000 
#define MAX_TILT_ANGLE 45.0 

#endif