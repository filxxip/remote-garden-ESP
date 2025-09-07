// littlefs_manager.cpp
#include "littlefs_manager.hpp"

#include <esp_err.h>
#include <esp_littlefs.h>
#include <esp_log.h>

namespace {
constexpr const char* TAG = "LFS_MGR";
}

namespace LittleFsManager {

bool init() {
    ESP_LOGI(TAG, "Initializing LittleFS");

    esp_vfs_littlefs_conf_t conf = {
        .base_path = "/littlefs",
        .partition_label = "littlefs",
        .format_if_mount_failed = true,
        .dont_mount = false,
    };  // @TODO fix warning during code building

    esp_err_t ret = esp_vfs_littlefs_register(&conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "LittleFS mount failed: %s", esp_err_to_name(ret));
        return false;
    }

    size_t total = 0, used = 0;
    ret = esp_littlefs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get LittleFS info: %s", esp_err_to_name(ret));
        return true;
    }

    ESP_LOGI(
        TAG, "LittleFS mounted. base=%s label=%s total=%u bytes, used=%u bytes",
        conf.base_path, conf.partition_label, (unsigned)total, (unsigned)used);
    return true;
}

void deinit() {
    esp_err_t ret = esp_vfs_littlefs_unregister("littlefs");
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "LittleFS unmount error: %s", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "LittleFS deinitialized");
    }
}

}  // namespace LittleFsManager
