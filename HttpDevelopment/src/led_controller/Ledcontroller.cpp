#include "LedController.h"

LedController::LedController(gpio_num_t pin) : _pin(pin) {
    // GPIO pinini çıkış olarak yapılandır
    esp_rom_gpio_pad_select_gpio(_pin);
    gpio_set_direction(_pin, GPIO_MODE_OUTPUT);
}

void LedController::on() {
    gpio_set_level(_pin, 1); // LED'i yak
}

void LedController::off() {
    gpio_set_level(_pin, 0); // LED'i söndür
}