#include "heating_mat_client_mock.hpp"

#include <esp_log.h>

static constexpr const char* TAG = "HeatingMatClientMock";

HeatingMatClientMock::HeatingMatClientMock(
    std::shared_ptr<MqttClient> mqttClient, int gardenId)
    : ActuatorClient(std::move(mqttClient), "heating_mat", gardenId) {}

bool HeatingMatClientMock::performTurnOn() {
    ESP_LOGI(TAG, "[MOCK] Heating mat turned ON");
    return true;
}

bool HeatingMatClientMock::performTurnOff() {
    ESP_LOGI(TAG, "[MOCK] Heating mat turned OFF");
    return true;
}