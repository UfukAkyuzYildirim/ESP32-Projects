#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "esp_http_server.h"
#include "../led_controller/LedController.h"

// HttpServer sınıfı
class HttpServer {
public:
    HttpServer();
    ~HttpServer();

    void start();
    void stop();

    // Root endpoint handler
    static esp_err_t handle_root(httpd_req_t *req);

    // LED kontrol endpoint'leri
    static esp_err_t handle_led_on(httpd_req_t *req);
    static esp_err_t handle_led_off(httpd_req_t *req);

    // Motor kontrol endpoint'leri
    static esp_err_t handle_motor_right(httpd_req_t *req);
    static esp_err_t handle_motor_left(httpd_req_t *req);
    static esp_err_t handle_motor_stop(httpd_req_t *req);

private:
    httpd_handle_t server_handle;
    LedController led;
};

#endif // HTTP_SERVER_H