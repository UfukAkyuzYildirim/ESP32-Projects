#include "http_server/HttpServer.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"

static const char *TAG = "MAIN";

// Wi-Fi ağı bilgileri
#define WIFI_SSID "witaminsiz"
#define WIFI_PASS "Abuzer123"

// Wi-Fi'ye bağlanma fonksiyonu
void wifi_init() {
    ESP_LOGI(TAG, "Wi-Fi başlatılıyor...");
    esp_err_t ret = nvs_flash_init(); // NVS Flash başlatılır
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init()); // Ağ arayüzü başlatılır
    ESP_ERROR_CHECK(esp_event_loop_create_default()); // Olay döngüsü başlatılır
    esp_netif_create_default_wifi_sta(); // Wi-Fi İstemcisi (Station) ayarlanır

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {};
    strcpy((char *)wifi_config.sta.ssid, WIFI_SSID);
    strcpy((char *)wifi_config.sta.password, WIFI_PASS);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wi-Fi ağına bağlanıyor...");
    ESP_ERROR_CHECK(esp_wifi_connect());
}

extern "C" void app_main() {
    ESP_LOGI(TAG, "Uygulama başlatılıyor...");

    // Wi-Fi bağlantısını başlat
    wifi_init();

    // HTTP Sunucusunu başlat
    HttpServer httpServer;
    httpServer.start();

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
