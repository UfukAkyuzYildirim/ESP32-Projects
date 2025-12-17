#pragma once

#include "driver/gpio.h"

class StatusLed {
public:
    explicit StatusLed(gpio_num_t pin);

    void init();
    void on();
    void off();

private:
    gpio_num_t pin_;
};
