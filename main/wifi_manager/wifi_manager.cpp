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
}  // namespace

void WifiManager::init() {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_country_t country = {"PL", 1, 13, WIFI_COUNTRY_POLICY_MANUAL};
    esp_wifi_set_country(&country);

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &WifiManager::onGotIp, this, nullptr));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &WifiManager::onDisconnected,
        this, nullptr));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

bool WifiManager::connect(const std::string& ssid, const std::string& pass,
                          int retries) {
    ESP_LOGI(TAG, "Try to connect to wifi with ssid: %s pass: %s", ssid.c_str(),
             pass.c_str());
    pendingNewCredentials = true;

    wifi_config_t wifi_config{};
    strncpy((char*)wifi_config.sta.ssid, ssid.c_str(),
            sizeof(wifi_config.sta.ssid));
    strncpy((char*)wifi_config.sta.password, pass.c_str(),
            sizeof(wifi_config.sta.password));

    esp_wifi_disconnect();
    vTaskDelay(pdMS_TO_TICKS(100));

    esp_err_t err = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_set_config failed: %s", esp_err_to_name(err));
        return false;
    }

    pendingNewCredentials = false;
    return esp_wifi_connect() == ESP_OK;
}

void WifiManager::disconnect() {
    if (!apReceived) {
        ESP_LOGW(TAG, "AP was not received, cannot disconnect");
        return;
    }

    manualDisconnect = true;

    wifi_config_t wifi_config{};
    esp_err_t err = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to clear Wi-Fi config: %s", esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "Wi-Fi credentials cleared");
    }

    if (auto err = esp_wifi_disconnect(); err == ESP_OK) {
        apReceived = false;
        ESP_LOGI(TAG, "Wi-Fi disconnected");
    } else if (err == ESP_ERR_WIFI_NOT_CONNECT) {
        ESP_LOGW(TAG, "Wi-Fi was not connected");
    } else {
        ESP_LOGE(TAG, "Failed to disconnect Wi-Fi: %s", esp_err_to_name(err));
    }
}

float WifiManager::getRssi() const {
    wifi_ap_record_t ap_info;
    if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
        return static_cast<float>(ap_info.rssi);
    } else {
        return -100.0f;
    }
}

void WifiManager::registerOnConnected(std::function<void()> callback) {
    onConnectedCallback = std::move(callback);
}

void WifiManager::onGotIp(void* arg, esp_event_base_t base, int32_t id,
                          void* data) {
    auto* self = static_cast<WifiManager*>(arg);
    self->apReceived = true;
    auto* event = static_cast<ip_event_got_ip_t*>(data);
    ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));

    if (self->onConnectedCallback) {
        self->onConnectedCallback();
    }
}

std::string WifiManager::getBssid() const {
    wifi_ap_record_t ap_info;
    if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
        char bssid_str[18];
        snprintf(bssid_str, sizeof(bssid_str), "%02X:%02X:%02X:%02X:%02X:%02X",
                 ap_info.bssid[0], ap_info.bssid[1], ap_info.bssid[2],
                 ap_info.bssid[3], ap_info.bssid[4], ap_info.bssid[5]);
        return std::string(bssid_str);
    } else {
        ESP_LOGW(TAG, "BSSID not available (not connected?)");
        return {};
    }
}

void WifiManager::onDisconnected(void* arg, esp_event_base_t base, int32_t id,
                                 void* data) {
    auto* self = static_cast<WifiManager*>(arg);
    ESP_LOGW(TAG, "Disconnected from Wi-Fi");

    if (self->manualDisconnect) {
        ESP_LOGI(TAG, "Manual disconnect, not retrying");
        self->manualDisconnect = false;
        return;
    }

    if (!self->pendingNewCredentials) {
        ESP_LOGI(TAG, "Retrying Wi-Fi connect...");
        esp_wifi_connect();
    } else {
        ESP_LOGI(TAG, "Waiting for new credentials, not retrying");
    }
}
