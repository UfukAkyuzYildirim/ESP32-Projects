#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include "driver/gpio.h" // GPIO için gerekli başlık dosyası

class LedController {
public:
    explicit LedController(gpio_num_t pin);
    void on();
    void off();

private:
    gpio_num_t _pin;
};

#endif // LED_CONTROLLER_H