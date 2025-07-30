#include "device_control_handler.hpp"

#include <esp_log.h>

#include <optional>

#include "utils/json_utils/json_utils.hpp"

namespace {

enum class ControlActionType {
    WATER_ON,
    WATER_OFF,
    ATOMIZE_ON,
    ATOMIZE_OFF,
    FAN_ON,
    FAN_OFF,
    HEATING_MAT_ON,
    HEATING_MAT_OFF
};

constexpr const char* TAG = "DeviceControlHandler";

}  // namespace

DeviceControlHandler::DeviceControlHandler(
    std::unique_ptr<ActuatorClient> atomize,
    std::unique_ptr<ActuatorClient> fan,
    std::unique_ptr<ActuatorClient> heatingMat,
    std::unique_ptr<ActuatorClient> water)
    : atomize(std::move(atomize)),
      fan(std::move(fan)),
      heatingMat(std::move(heatingMat)),
      water(std::move(water)) {}

void DeviceControlHandler::operate(const std::string& topic,
                                   const std::string& payload) {
    ESP_LOGI(TAG, "Received message on %s: %s", topic.c_str(), payload.c_str());

    if (auto idOpt = JsonUtils::getInt(payload, {"action", "id"});
        idOpt.has_value()) {
        ControlActionType actionType =
            static_cast<ControlActionType>(idOpt.value());

        switch (actionType) {
            case ControlActionType::WATER_ON:
                water->turnOn();
                break;
            case ControlActionType::WATER_OFF:
                water->turnOff();
                break;
            case ControlActionType::ATOMIZE_ON:
                atomize->turnOn();
                break;
            case ControlActionType::ATOMIZE_OFF:
                atomize->turnOff();
                break;
            case ControlActionType::FAN_ON:
                fan->turnOn();
                break;
            case ControlActionType::FAN_OFF:
                fan->turnOff();
                break;
            case ControlActionType::HEATING_MAT_ON:
                heatingMat->turnOn();
                break;
            case ControlActionType::HEATING_MAT_OFF:
                heatingMat->turnOff();
                break;
            default:
                ESP_LOGW(TAG, "Unknown action ID: %d", idOpt.value());
                break;
        }
    } else {
        ESP_LOGE(TAG, "Missing or invalid action.id field");
    }
}
