#include "atomize_client_mock.hpp"

#include <esp_log.h>

static constexpr const char* TAG = "AtomizeClientMock";

AtomizeClientMock::AtomizeClientMock(std::shared_ptr<MqttClient> mqttClient,
                                     int gardenId)
    : ActuatorClient(std::move(mqttClient), "atomize", gardenId) {}

bool AtomizeClientMock::performTurnOn() {
    ESP_LOGI(TAG, "[MOCK] Atomizer turned ON");
    return true;
}

bool AtomizeClientMock::performTurnOff() {
    ESP_LOGI(TAG, "[MOCK] Atomizer turned OFF");
    return true;
}