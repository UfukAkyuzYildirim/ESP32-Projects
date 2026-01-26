#include "HttpServer.h"

#include "Bts7960Driver.h"

#include "esp_log.h"

#include <cstring>
#include <cstdlib>

namespace {
bool starts_with(const char *text, const char *prefix) {
    if (text == nullptr || prefix == nullptr) {
        return false;
    }
    const size_t prefix_len = std::strlen(prefix);
    return std::strncmp(text, prefix, prefix_len) == 0;
}
} // namespace

static const char *kTag = "HttpServer";

HttpServer::HttpServer(Bts7960Driver *motor) : motor_(motor) {}

HttpServer::~HttpServer() {
    stop();
}

void HttpServer::start() {
    if (server_handle_ != nullptr) {
        return;
    }

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    if (httpd_start(&server_handle_, &config) != ESP_OK) {
        ESP_LOGE(kTag, "Failed to start HTTP server");
        server_handle_ = nullptr;
        return;
    }

    ESP_LOGI(kTag, "HTTP server started");

    httpd_uri_t root_uri{
        .uri = "/",
        .method = HTTP_GET,
        .handler = &HttpServer::handle_root,
        .user_ctx = nullptr,
    };
    httpd_register_uri_handler(server_handle_, &root_uri);

    httpd_uri_t ws_uri{
        .uri = "/ws",
        .method = HTTP_GET,
        .handler = &HttpServer::handle_ws,
        .user_ctx = motor_,
        .is_websocket = true,
    };
    httpd_register_uri_handler(server_handle_, &ws_uri);

    httpd_uri_t motor_right_uri{
        .uri = "/motor/right",
        .method = HTTP_GET,
        .handler = &HttpServer::handle_motor_right,
        .user_ctx = motor_,
    };
    httpd_register_uri_handler(server_handle_, &motor_right_uri);

    httpd_uri_t motor_left_uri{
        .uri = "/motor/left",
        .method = HTTP_GET,
        .handler = &HttpServer::handle_motor_left,
        .user_ctx = motor_,
    };
    httpd_register_uri_handler(server_handle_, &motor_left_uri);

    httpd_uri_t motor_stop_uri{
        .uri = "/motor/stop",
        .method = HTTP_GET,
        .handler = &HttpServer::handle_motor_stop,
        .user_ctx = motor_,
    };
    httpd_register_uri_handler(server_handle_, &motor_stop_uri);
}

void HttpServer::stop() {
    if (server_handle_ == nullptr) {
        return;
    }

    httpd_stop(server_handle_);
    server_handle_ = nullptr;
    ESP_LOGI(kTag, "HTTP server stopped");
}

esp_err_t HttpServer::handle_root(httpd_req_t *req) {
    const char *response =
        "RemoteCarMotorControl API\n"
        "GET /motor/right?duty=0-100\n"
        "GET /motor/left?duty=0-100\n"
        "GET /motor/stop\n";

    const char *response_ws =
        "\nWebSocket:\n"
        "GET /ws (upgrade to WebSocket)\n"
        "Send text frames: '67' (right), 'right:67', 'left:30', 'stop'\n";

    httpd_resp_set_type(req, "text/plain");
    httpd_resp_sendstr_chunk(req, response);
    httpd_resp_sendstr_chunk(req, response_ws);
    httpd_resp_sendstr_chunk(req, nullptr);
    return ESP_OK;
}

uint32_t HttpServer::map_duty_percent_to_pwm(uint32_t duty_percent) {
    if (duty_percent > 100) {
        duty_percent = 100;
    }

    constexpr uint32_t kMaxDuty = (1u << 8) - 1u; // 255
    return (kMaxDuty * duty_percent) / 100u;
}

bool HttpServer::try_read_duty_percent(httpd_req_t *req, uint32_t *outDuty) {
    if (outDuty == nullptr) {
        return false;
    }

    char query[64];
    size_t query_len = httpd_req_get_url_query_len(req) + 1;
    if (query_len <= 1 || query_len > sizeof(query)) {
        return false;
    }

    if (httpd_req_get_url_query_str(req, query, query_len) != ESP_OK) {
        return false;
    }

    char duty_str[8];
    if (httpd_query_key_value(query, "duty", duty_str, sizeof(duty_str)) != ESP_OK) {
        return false;
    }

    const int duty_percent = std::atoi(duty_str);
    if (duty_percent < 0 || duty_percent > 100) {
        return false;
    }

    *outDuty = map_duty_percent_to_pwm(static_cast<uint32_t>(duty_percent));
    return true;
}

esp_err_t HttpServer::handle_motor_right(httpd_req_t *req) {
    auto *motor = static_cast<Bts7960Driver *>(req->user_ctx);
    if (motor == nullptr) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Motor not configured");
        return ESP_FAIL;
    }

    uint32_t duty = 0;
    if (!try_read_duty_percent(req, &duty)) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing/invalid duty (0-100)");
        return ESP_ERR_INVALID_ARG;
    }

    motor->turnRight(duty);
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_send(req, "OK\n", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t HttpServer::handle_motor_left(httpd_req_t *req) {
    auto *motor = static_cast<Bts7960Driver *>(req->user_ctx);
    if (motor == nullptr) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Motor not configured");
        return ESP_FAIL;
    }

    uint32_t duty = 0;
    if (!try_read_duty_percent(req, &duty)) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing/invalid duty (0-100)");
        return ESP_ERR_INVALID_ARG;
    }

    motor->turnLeft(duty);
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_send(req, "OK\n", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t HttpServer::handle_motor_stop(httpd_req_t *req) {
    auto *motor = static_cast<Bts7960Driver *>(req->user_ctx);
    if (motor == nullptr) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Motor not configured");
        return ESP_FAIL;
    }

    motor->stop();
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_send(req, "OK\n", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t HttpServer::handle_ws(httpd_req_t *req) {
    // Handshake happens via HTTP GET.
    if (req->method == HTTP_GET) {
        ESP_LOGI(kTag, "WebSocket handshake completed");
        return ESP_OK;
    }

    auto *motor = static_cast<Bts7960Driver *>(req->user_ctx);
    if (motor == nullptr) {
        return ESP_FAIL;
    }

    httpd_ws_frame_t ws_pkt;
    std::memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;

    // First get frame length.
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK) {
        ESP_LOGW(kTag, "httpd_ws_recv_frame (len) failed: %d", ret);
        return ret;
    }

    if (ws_pkt.len == 0) {
        return ESP_OK;
    }

    char *payload = static_cast<char *>(std::malloc(ws_pkt.len + 1));
    if (payload == nullptr) {
        return ESP_ERR_NO_MEM;
    }

    ws_pkt.payload = reinterpret_cast<uint8_t *>(payload);
    ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
    if (ret != ESP_OK) {
        std::free(payload);
        ESP_LOGW(kTag, "httpd_ws_recv_frame (payload) failed: %d", ret);
        return ret;
    }
    payload[ws_pkt.len] = '\0';

    ESP_LOGI(kTag, "Received WS payload (%u bytes)", static_cast<unsigned>(ws_pkt.len));
    ESP_LOG_BUFFER_HEXDUMP(kTag, payload, ws_pkt.len, ESP_LOG_INFO);

    // Supported payloads (text):
    // - "67" => right:67
    // - "right:67" / "left:30"
    // - "stop" (or "0")
    const char *text = payload;
    while (*text == ' ' || *text == '\t' || *text == '\n' || *text == '\r') {
        ++text;
    }

    if (starts_with(text, "stop") || std::strcmp(text, "0") == 0) {
        motor->stop();
        std::free(payload);
        return ESP_OK;
    }

    bool is_left = false;
    if (starts_with(text, "right:")) {
        text += std::strlen("right:");
    } else if (starts_with(text, "left:")) {
        text += std::strlen("left:");
        is_left = true;
    }

    const int duty_percent = std::atoi(text);
    if (duty_percent <= 0) {
        motor->stop();
        std::free(payload);
        return ESP_OK;
    }

    const uint32_t pwm = map_duty_percent_to_pwm(static_cast<uint32_t>(duty_percent));
    if (is_left) {
        motor->turnLeft(pwm);
    } else {
        motor->turnRight(pwm);
    }

    std::free(payload);
    return ESP_OK;
}
