#include "garden_specifier_handler.hpp"

#include <esp_log.h>

#include "utils/json_utils/json_utils.hpp"

namespace {
constexpr const char* TAG = "GardenSpecifierHandler";
}

void GardenSpecifierHandler::operate(const std::string& topic,
                                     const std::string& payload) {
    ESP_LOGI(TAG, "Received message on %s: %s", topic.c_str(), payload.c_str());

    if (auto idOpt = JsonUtils::getInt(payload, {"garden", "id"});
        idOpt.has_value()) {
        auto val = idOpt.value();
        ESP_LOGI("GardenSpecifierHandler", "Extracted garden ID: %d", val);

        if (callback) {
            ESP_LOGI(TAG, "Garden specified callback will be executed");
            callback(garden, val);
        }

        garden = std::make_optional(val);
    } else {
        ESP_LOGE(TAG, "Missing or invalid garden.id field");
    }
}

void GardenSpecifierHandler::setCallback(
    std::function<void(const std::optional<int>&, int)> cb) {
    callback = std::move(cb);
}