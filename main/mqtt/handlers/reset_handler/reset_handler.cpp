#include "reset_handler.hpp"

#include <esp_log.h>

#include "utils/device_provisioner/device_provisioner.hpp"
#include "utils/file_manager/file_manager.hpp"

namespace {
constexpr const char* TAG = "ResetHandler";
}

ResetHandler::ResetHandler(std::shared_ptr<WifiManager> wifi,
                           std::shared_ptr<WifiManager> mqtt)
    : wifi(std::move(wifi)), mqtt(std::move(mqtt)) {}

void ResetHandler::operate(const std::string& topic,
                           const std::string& payload) {
    ESP_LOGI(TAG, "Received message on %s: %s", topic.c_str(), payload.c_str());

    if (DeviceProvisioner::removeProvisioned()) {
        ESP_LOGI(TAG, "Provisioning data removed successfully");
    } else {
        ESP_LOGW(TAG, "Failed to remove some provisioning files");
    }

    mqtt->stop();
    vTaskDelay(pdMS_TO_TICKS(600));
    wifi->disconnect();
    vTaskDelay(pdMS_TO_TICKS(600));
}
