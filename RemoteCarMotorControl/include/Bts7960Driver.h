#pragma once

#include <cstdint>

#include "driver/gpio.h"

class Bts7960Driver {
public:
    Bts7960Driver(gpio_num_t rightPwmPin, gpio_num_t leftPwmPin, gpio_num_t rightEnablePin, gpio_num_t leftEnablePin);

    // Call once at startup
    void init();

    // Duty is 0..255 for 8-bit PWM
    void turnRight(uint32_t duty);
    void turnLeft(uint32_t duty);
    void stop();

private:
    gpio_num_t rightPwmPin_;
    gpio_num_t leftPwmPin_;
    gpio_num_t rightEnablePin_;
    gpio_num_t leftEnablePin_;

    static uint32_t clampDuty(uint32_t duty);
};
