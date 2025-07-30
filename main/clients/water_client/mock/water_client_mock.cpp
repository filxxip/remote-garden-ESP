#include "water_client_mock.hpp"

#include <esp_log.h>

static constexpr const char* TAG = "WaterClientMock";

WaterClientMock::WaterClientMock(std::shared_ptr<MqttClient> mqttClient,
                                 int gardenId)
    : ActuatorClient(std::move(mqttClient), "water", gardenId) {}

bool WaterClientMock::performTurnOn() {
    ESP_LOGI(TAG, "[MOCK] Water turned ON");
    return true;
}

bool WaterClientMock::performTurnOff() {
    ESP_LOGI(TAG, "[MOCK] Water turned OFF");
    return true;
}
