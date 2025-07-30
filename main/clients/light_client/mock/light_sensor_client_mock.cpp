#include "light_sensor_client_mock.hpp"

LightSensorClientMock::LightSensorClientMock(
    std::shared_ptr<MqttClient> mqttClient, int gardenId)
    : SensorClient(std::move(mqttClient), "light", gardenId) {}

float LightSensorClientMock::readValue() { return 300.0f; }
