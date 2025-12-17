#include "Config.h"

#include "Bts7960Driver.h"
#include "HttpServer.h"
#include "StatusLed.h"
#include "WifiStation.h"

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *kTag = "RemoteCar";

extern "C" void app_main(void) {
    ESP_LOGI(kTag, "Booting RemoteCarMotorControl (ESP-IDF + C++)");

    StatusLed led(config::kStatusLedPin);
    led.init();

    ESP_LOGI(kTag, "Boot OK. LED blink once.");
    led.on();
    vTaskDelay(pdMS_TO_TICKS(200));
    led.off();

    Bts7960Driver motor(config::kRightPwmPin, config::kLeftPwmPin, config::kRightEnablePin, config::kLeftEnablePin);
    motor.init();

    motor.stop();

    ESP_LOGI(kTag, "Starting WiFi (STA)");
    wifi_station::init();

    ESP_LOGI(kTag, "Starting HTTP server");
    HttpServer server(&motor);
    server.start();

    ESP_LOGI(kTag, "Ready. Try: http://<esp-ip>/motor/right?duty=40");

    // Keep task alive without hogging CPU.
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
