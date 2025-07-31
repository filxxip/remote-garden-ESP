#include "light_sensor_client_mock.hpp"

#include "clients/sensors/sensor_type.hpp"

LightSensorClientMock::LightSensorClientMock(
    std::shared_ptr<MqttClient> mqttClient, const int gardenId)
    : SensorClient(std::move(mqttClient), SensorType::Light, gardenId) {}

float LightSensorClientMock::readValue() { return 300.0f; }
