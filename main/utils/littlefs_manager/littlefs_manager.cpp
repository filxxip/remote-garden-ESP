#include "littlefs_manager.hpp"

#include <esp_littlefs.h>
#include <esp_log.h>

namespace {
constexpr const char* TAG = "LITTLEFS";
}  // namespace

namespace LittlefsManager {

bool init() {
    ESP_LOGI(TAG, "Initializing LittleFS");

    esp_vfs_littlefs_conf_t conf = {.base_path = "/littlefs",
                                    .partition_label = "storage",
                                    .partition = nullptr,
                                    .format_if_mount_failed = true,
                                    .read_only = false,
                                    .dont_mount = false,
                                    .grow_on_mount = false};

    esp_err_t ret = esp_vfs_littlefs_register(&conf);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find LittleFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize LittleFS (%s)",
                     esp_err_to_name(ret));
        }
        return false;
    }

    size_t total = 0, used = 0;
    ret = esp_littlefs_info("storage", &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get LittleFS partition information (%s)",
                 esp_err_to_name(ret));
        return false;
    }

    ESP_LOGI(TAG,
             "Successfully initialized. Partition size: total: %d bytes, used: "
             "%d bytes",
             total, used);
    return true;
}

void deinit() {
    esp_vfs_littlefs_unregister("storage");
    ESP_LOGI(TAG, "LittleFS deinitialized");
}

}  // namespace LittlefsManager
