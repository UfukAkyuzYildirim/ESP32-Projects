#include "HttpServer.h"
#include "esp_log.h"
// #include "../motor_controller/MotorController.h"
#include "../motorControl_controller/MotorController.h"

// Logger TAG
static const char *TAG = "HttpServer";

// Motor kontrol nesnesi
static MotorController motor(GPIO_NUM_26, GPIO_NUM_14, GPIO_NUM_25, GPIO_NUM_27);

HttpServer::HttpServer() 
    : server_handle(nullptr), led(GPIO_NUM_2) { // Tanımlama sırasına göre inisyalize edildi
    ESP_LOGI(TAG, "HTTP Server nesnesi oluşturuldu.");
    motor.init(); // Motor kontrolünü başlat
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

        // Motor Right endpoint
        httpd_uri_t motor_right_uri = {
            .uri = "/motor/right",
            .method = HTTP_GET,
            .handler = handle_motor_right,
            .user_ctx = nullptr
        };
        httpd_register_uri_handler(server_handle, &motor_right_uri);

        // Motor Left endpoint
        httpd_uri_t motor_left_uri = {
            .uri = "/motor/left",
            .method = HTTP_GET,
            .handler = handle_motor_left,
            .user_ctx = nullptr
        };
        httpd_register_uri_handler(server_handle, &motor_left_uri);

        // Motor Stop endpoint
        httpd_uri_t motor_stop_uri = {
            .uri = "/motor/stop",
            .method = HTTP_GET,
            .handler = handle_motor_stop,
            .user_ctx = nullptr
        };
        httpd_register_uri_handler(server_handle, &motor_stop_uri);
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
    const char *response = "Hello, ESP32! Use /led/on, /led/off, /motor/right, /motor/left, or /motor/stop to control the devices.";
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

esp_err_t HttpServer::handle_motor_right(httpd_req_t *req) {
    // "duty" parametresini al
    char query[64];
    size_t query_len = httpd_req_get_url_query_len(req) + 1;
    if (query_len > 1 && httpd_req_get_url_query_str(req, query, query_len) == ESP_OK) {
        char duty_str[8];
        if (httpd_query_key_value(query, "duty", duty_str, sizeof(duty_str)) == ESP_OK) {
            int duty = atoi(duty_str); // String'den integer'a çevir
            if (duty >= 0 && duty <= 100) {
                motor.turnRight(duty); // Motor sağa döner
                httpd_resp_send(req, "Motor turning right", HTTPD_RESP_USE_STRLEN);
                return ESP_OK;
            }
        }
    }
    httpd_resp_send(req, "Invalid or missing 'duty' parameter", HTTPD_RESP_USE_STRLEN);
    return ESP_ERR_INVALID_ARG;
}

esp_err_t HttpServer::handle_motor_left(httpd_req_t *req) {
    // "duty" parametresini al
    char query[64];
    size_t query_len = httpd_req_get_url_query_len(req) + 1;
    if (query_len > 1 && httpd_req_get_url_query_str(req, query, query_len) == ESP_OK) {
        char duty_str[8];
        if (httpd_query_key_value(query, "duty", duty_str, sizeof(duty_str)) == ESP_OK) {
            int duty = atoi(duty_str); // String'den integer'a çevir
            if (duty >= 0 && duty <= 100) {
                motor.turnLeft(duty); // Motor sola döner
                httpd_resp_send(req, "Motor turning left", HTTPD_RESP_USE_STRLEN);
                return ESP_OK;
            }
        }
    }
    httpd_resp_send(req, "Invalid or missing 'duty' parameter", HTTPD_RESP_USE_STRLEN);
    return ESP_ERR_INVALID_ARG;
}

esp_err_t HttpServer::handle_motor_stop(httpd_req_t *req) {
    motor.stop(); // Motor durdurulur
    httpd_resp_send(req, "Motor stopped", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}