#include "fan_client_mock.hpp"

#include <esp_log.h>

static constexpr const char* TAG = "FanClientMock";

FanClientMock::FanClientMock(std::shared_ptr<MqttClient> mqttClient,
                             int gardenId)
    : ActuatorClient(std::move(mqttClient), "fan", gardenId) {}

bool FanClientMock::performTurnOn() {
    ESP_LOGI(TAG, "[MOCK] Fan turned ON");
    return true;
}

bool FanClientMock::performTurnOff() {
    ESP_LOGI(TAG, "[MOCK] Fan turned OFF");
    return true;
}
