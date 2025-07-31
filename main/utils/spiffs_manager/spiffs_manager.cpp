#include "spiffs_manager.hpp"

#include <esp_log.h>
#include <esp_spiffs.h>

namespace {
constexpr const char* TAG = "SPIFFS";
}

namespace SpiffsManager {

bool init() {
    ESP_LOGI(TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {.base_path = "/spiffs",
                                  .partition_label = "spiffs",
                                  .max_files = 5,
                                  .format_if_mount_failed = true};

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)",
                     esp_err_to_name(ret));
        }
        return false;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info("spiffs", &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)",
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
    esp_vfs_spiffs_unregister("spiffs");
    ESP_LOGI(TAG, "SPIFFS deinitialized");
}

}  // namespace SpiffsManager