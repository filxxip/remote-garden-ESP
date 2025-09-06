#include "resume_handler.hpp"

#include <esp_log.h>

#include "utils/device_provisioner/device_provisioner.hpp"
#include "utils/file_manager/file_manager.hpp"
#include "utils/nvs_manager/nvs_credentials_manager.hpp"

namespace {
constexpr const char* TAG = "ResumeHandler";
}

ResumeHandler::ResumeHandler(
    std::array<std::shared_ptr<SensorPollingManager>, 2> sensors)
    : sensors(std::move(sensors)) {}

void ResumeHandler::operate(const std::string& topic,
                            const std::string& payload) {
    ESP_LOGI(TAG, "Received message on %s: %s", topic.c_str(), payload.c_str());

    for (auto& sensor : sensors) {
        if (sensor) {
            sensor->resume();
        }
    }
}
