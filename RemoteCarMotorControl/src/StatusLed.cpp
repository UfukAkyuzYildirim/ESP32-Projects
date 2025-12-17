#include "StatusLed.h"

#include "esp_err.h"

StatusLed::StatusLed(gpio_num_t pin) : pin_(pin) {}

void StatusLed::init() {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << pin_),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    off();
}

void StatusLed::on() {
    ESP_ERROR_CHECK(gpio_set_level(pin_, 1));
}

void StatusLed::off() {
    ESP_ERROR_CHECK(gpio_set_level(pin_, 0));
}
