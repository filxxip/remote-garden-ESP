#include "mock_actuator_client.hpp"

#include <esp_log.h>

#include "utils/enum/enum_mapper.hpp"

namespace {
constexpr const char* TAG = "MockActuatorClient";
}

bool MockActuatorClient::performTurnOn() {
    ESP_LOGI(TAG, "[MOCK] %s turned ON", enumToString(type).c_str());
    return true;
}

bool MockActuatorClient::performTurnOff() {
    ESP_LOGI(TAG, "[MOCK] %s turned OFF", enumToString(type).c_str());
    return true;
}