#include "WifiStation.h"

#include "Config.h"

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_netif_ip_addr.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

#include "lwip/ip4_addr.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include <cstring>

static const char *kTag = "WiFi";

namespace {

EventGroupHandle_t g_event_group = nullptr;
constexpr EventBits_t kWifiConnectedBit = BIT0;
constexpr EventBits_t kIpGotBit = BIT1;

void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    (void)arg;

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(kTag, "WiFi STA started; connecting...");
        esp_wifi_connect();
        return;
    }

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
        xEventGroupSetBits(g_event_group, kWifiConnectedBit);
        ESP_LOGI(kTag, "WiFi connected");
        return;
    }

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        auto *disconnected = static_cast<wifi_event_sta_disconnected_t *>(event_data);
        ESP_LOGW(kTag, "WiFi disconnected (reason=%d); retrying...", disconnected ? disconnected->reason : -1);
        xEventGroupClearBits(g_event_group, kWifiConnectedBit | kIpGotBit);
        esp_wifi_connect();
        return;
    }

    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(g_event_group, kWifiConnectedBit | kIpGotBit);
        auto *event = static_cast<ip_event_got_ip_t *>(event_data);
        if (event != nullptr) {
            ESP_LOGI(kTag, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        } else {
            ESP_LOGI(kTag, "Got IP");
        }
        return;
    }
}

} // namespace

namespace wifi_station {

void init() {
    ESP_LOGI(kTag, "Initializing WiFi (STA)");

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();

    if (g_event_group == nullptr) {
        g_event_group = xEventGroupCreate();
    }

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, nullptr, nullptr));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, nullptr, nullptr));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config{};
    std::strncpy(reinterpret_cast<char *>(wifi_config.sta.ssid), config::kWifiSsid, sizeof(wifi_config.sta.ssid));
    std::strncpy(reinterpret_cast<char *>(wifi_config.sta.password), config::kWifiPass, sizeof(wifi_config.sta.password));

    if (config::kUseStaticIp) {
        if (sta_netif == nullptr) {
            ESP_LOGE(kTag, "Failed to create STA netif; cannot set static IP");
        } else {
            esp_netif_ip_info_t ip_info{};
            IP4_ADDR(&ip_info.ip, config::kStaticIp[0], config::kStaticIp[1], config::kStaticIp[2], config::kStaticIp[3]);
            IP4_ADDR(&ip_info.gw, config::kStaticGateway[0], config::kStaticGateway[1], config::kStaticGateway[2], config::kStaticGateway[3]);
            IP4_ADDR(&ip_info.netmask, config::kStaticNetmask[0], config::kStaticNetmask[1], config::kStaticNetmask[2], config::kStaticNetmask[3]);

            ESP_ERROR_CHECK(esp_netif_dhcpc_stop(sta_netif));
            ESP_ERROR_CHECK(esp_netif_set_ip_info(sta_netif, &ip_info));

            esp_netif_dns_info_t dns{};
            IP4_ADDR(&dns.ip.u_addr.ip4, config::kStaticDns[0], config::kStaticDns[1], config::kStaticDns[2], config::kStaticDns[3]);
            dns.ip.type = ESP_IPADDR_TYPE_V4;
            ESP_ERROR_CHECK(esp_netif_set_dns_info(sta_netif, ESP_NETIF_DNS_MAIN, &dns));

            ESP_LOGI(
                kTag,
                "Static IP configured: " IPSTR " (gw=" IPSTR ", mask=" IPSTR ")",
                IP2STR(&ip_info.ip),
                IP2STR(&ip_info.gw),
                IP2STR(&ip_info.netmask));
        }
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    // Wait briefly for connectivity, but don't block forever.
    constexpr TickType_t kWait = pdMS_TO_TICKS(15000);
    const EventBits_t wait_for = config::kUseStaticIp ? kWifiConnectedBit : kIpGotBit;
    EventBits_t bits = xEventGroupWaitBits(g_event_group, wait_for, pdFALSE, pdTRUE, kWait);

    if (config::kUseStaticIp) {
        if ((bits & kWifiConnectedBit) == 0) {
            ESP_LOGW(kTag, "Not connected yet (check SSID/password). Continuing anyway.");
        }
    } else {
        if ((bits & kIpGotBit) == 0) {
            ESP_LOGW(kTag, "No IP yet (check SSID/password). Continuing anyway.");
        }
    }
}

} // namespace wifi_station
