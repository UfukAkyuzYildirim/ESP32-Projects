#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "esp_http_server.h"
#include "../led_controller/LedController.h" // LedController sınıfını dahil ediyoruz

class HttpServer {
public:
    HttpServer();
    ~HttpServer();

    void start();
    void stop();

private:
    httpd_handle_t server_handle; // HTTP sunucu handle
    LedController led;            // LED kontrol sınıfı

    static esp_err_t handle_root(httpd_req_t *req); // Root URI için handler
    static esp_err_t handle_led_on(httpd_req_t *req); // LED On URI için handler
    static esp_err_t handle_led_off(httpd_req_t *req); // LED Off URI için handler
};

#endif // HTTP_SERVER_H
