#include "nvs_manager.hpp"

#include <nvs.h>
#include <nvs_flash.h>

namespace {
constexpr const char* TAG = "NvsManager";
}

bool NvsManager::init() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init NVS: %s", esp_err_to_name(ret));
        return false;
    }
    ESP_LOGI(TAG, "NVS initialized");
    return true;
}

bool NvsManager::writeString(const std::string& namespaceName,
                             const std::string& key, const std::string& value) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open(namespaceName.c_str(), NVS_READWRITE, &handle);
    if (err != ESP_OK) return false;

    err = nvs_set_str(handle, key.c_str(), value.c_str());
    if (err == ESP_OK) err = nvs_commit(handle);

    nvs_close(handle);
    return err == ESP_OK;
}

bool NvsManager::readString(const std::string& namespaceName,
                            const std::string& key, std::string& outValue) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open(namespaceName.c_str(), NVS_READONLY, &handle);
    if (err != ESP_OK) return false;

    size_t required_size = 0;
    err = nvs_get_str(handle, key.c_str(), nullptr, &required_size);
    if (err != ESP_OK || required_size == 0) {
        nvs_close(handle);
        return false;
    }

    char* buffer = new char[required_size];
    err = nvs_get_str(handle, key.c_str(), buffer, &required_size);
    if (err == ESP_OK) {
        outValue = std::string(buffer);
    }
    delete[] buffer;
    nvs_close(handle);
    return err == ESP_OK;
}

bool NvsManager::writeInt(const std::string& namespaceName,
                          const std::string& key, int32_t value) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open(namespaceName.c_str(), NVS_READWRITE, &handle);
    if (err != ESP_OK) return false;

    err = nvs_set_i32(handle, key.c_str(), value);
    if (err == ESP_OK) err = nvs_commit(handle);

    nvs_close(handle);
    return err == ESP_OK;
}

bool NvsManager::readInt(const std::string& namespaceName,
                         const std::string& key, int32_t& outValue) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open(namespaceName.c_str(), NVS_READONLY, &handle);
    if (err != ESP_OK) return false;

    err = nvs_get_i32(handle, key.c_str(), &outValue);
    nvs_close(handle);
    return err == ESP_OK;
}
