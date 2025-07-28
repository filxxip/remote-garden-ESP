#include "wifi_manager.hpp"

#include <cstring>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"

namespace {
constexpr const char* TAG = "WifiManager";

bool check_connection() {
    wifi_ap_record_t ap_info;
    return esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK;
}
}  // namespace

void WifiManager::init() const {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
}

bool WifiManager::connect(const std::string& ssid, const std::string& password,
                          int max_retries) {
    wifi_config_t wifi_config = {};
    std::strncpy((char*)wifi_config.sta.ssid, ssid.c_str(),
                 sizeof(wifi_config.sta.ssid) - 1);
    std::strncpy((char*)wifi_config.sta.password, password.c_str(),
                 sizeof(wifi_config.sta.password) - 1);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());

    ESP_LOGI(TAG, "Connecting to SSID: %s", ssid.c_str());

    for (int retry = 1; retry <= max_retries; ++retry) {
        vTaskDelay(pdMS_TO_TICKS(2000));
        if (check_connection()) {
            ESP_LOGI(TAG, "Connected to WiFi");
            connected = true;
            return true;
        }
        ESP_LOGW(TAG, "Retry %d/%d...", retry, max_retries);
        esp_wifi_connect();
    }

    ESP_LOGE(TAG, "Failed to connect after %d retries", max_retries);
    connected = false;
    return false;
}

bool WifiManager::isConnected() const { return connected; }
