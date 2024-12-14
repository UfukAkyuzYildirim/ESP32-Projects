#include "HttpServer.h"
#include "esp_log.h"

static const char *TAG = "HttpServer";

HttpServer::HttpServer() 
    : server_handle(nullptr), led(GPIO_NUM_2) { // Tanımlama sırasına göre inisyalize edildi
    ESP_LOGI(TAG, "HTTP Server nesnesi oluşturuldu.");
}

HttpServer::~HttpServer() {
    stop();
}

void HttpServer::start() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    if (httpd_start(&server_handle, &config) == ESP_OK) {
        ESP_LOGI(TAG, "HTTP Server başlatıldı.");

        // Root endpoint handler
        httpd_uri_t root_uri = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = handle_root,
            .user_ctx = nullptr
        };
        httpd_register_uri_handler(server_handle, &root_uri);

        // LED On endpoint
        httpd_uri_t led_on_uri = {
            .uri = "/led/on",
            .method = HTTP_GET,
            .handler = handle_led_on,
            .user_ctx = &led
        };
        httpd_register_uri_handler(server_handle, &led_on_uri);

        // LED Off endpoint
        httpd_uri_t led_off_uri = {
            .uri = "/led/off",
            .method = HTTP_GET,
            .handler = handle_led_off,
            .user_ctx = &led
        };
        httpd_register_uri_handler(server_handle, &led_off_uri);
    } else {
        ESP_LOGE(TAG, "HTTP Server başlatılamadı.");
    }
}

void HttpServer::stop() {
    if (server_handle) {
        httpd_stop(server_handle);
        ESP_LOGI(TAG, "HTTP Server durduruldu.");
    }
}

esp_err_t HttpServer::handle_root(httpd_req_t *req) {
    const char *response = "Hello, ESP32! Use /led/on or /led/off to control the LED.";
    httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t HttpServer::handle_led_on(httpd_req_t *req) {
    LedController *led = static_cast<LedController *>(req->user_ctx); // LED kontrol sınıfını al
    led->on(); // LED'i yak
    httpd_resp_send(req, "LED is ON", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t HttpServer::handle_led_off(httpd_req_t *req) {
    LedController *led = static_cast<LedController *>(req->user_ctx); // LED kontrol sınıfını al
    led->off(); // LED'i söndür
    httpd_resp_send(req, "LED is OFF", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}
