#include "nvs_credentials_manager.hpp"

#include <esp_log.h>

#include "nvs_manager.hpp"

namespace {
constexpr const char* TAG = "NvsCredManager";
constexpr const char* NVS_NS = "cred";
constexpr const char* KEY_SSID = "ssid";
constexpr const char* KEY_PASS = "password";
constexpr const char* KEY_USER = "user_key";
}  // namespace

bool NvsCredentialsManager::save(const Credentials& creds) {
    if (!creds.valid()) {
        ESP_LOGW(TAG, "Cannot save invalid credentials");
        return false;
    }
    bool ok1 = NvsManager::writeString(NVS_NS, KEY_SSID, creds.ssid);
    bool ok2 = NvsManager::writeString(NVS_NS, KEY_PASS, creds.password);
    ESP_LOGI(TAG, "Credentials saved to NVS");
    return ok1 && ok2;
}

bool NvsCredentialsManager::load(Credentials& creds) {
    std::string ssid, pass;
    if (!NvsManager::readString(NVS_NS, KEY_SSID, ssid)) return false;
    if (!NvsManager::readString(NVS_NS, KEY_PASS, pass)) return false;

    creds.ssid = ssid;
    creds.password = pass;
    ESP_LOGI(TAG, "Credentials loaded from NVS");
    return creds.valid();
}

bool NvsCredentialsManager::clear() {
    bool ok1 = NvsManager::writeString(NVS_NS, KEY_SSID, "");
    bool ok2 = NvsManager::writeString(NVS_NS, KEY_PASS, "");
    ESP_LOGI(TAG, "Credentials cleared in NVS");
    return ok1 && ok2;
}
