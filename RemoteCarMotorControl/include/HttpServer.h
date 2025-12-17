#pragma once

#include "esp_http_server.h"

class Bts7960Driver;

class HttpServer {
public:
    explicit HttpServer(Bts7960Driver *motor);
    ~HttpServer();

    void start();
    void stop();

private:
    static esp_err_t handle_root(httpd_req_t *req);
    static esp_err_t handle_motor_right(httpd_req_t *req);
    static esp_err_t handle_motor_left(httpd_req_t *req);
    static esp_err_t handle_motor_stop(httpd_req_t *req);
    static esp_err_t handle_ws(httpd_req_t *req);

    static bool try_read_duty_percent(httpd_req_t *req, uint32_t *outDuty);
    static uint32_t map_duty_percent_to_pwm(uint32_t duty_percent);

    httpd_handle_t server_handle_{nullptr};
    Bts7960Driver *motor_{nullptr};
};
