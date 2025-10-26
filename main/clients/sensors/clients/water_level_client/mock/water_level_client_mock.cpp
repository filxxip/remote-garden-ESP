#include "water_level_client_mock.hpp"

#include "clients/sensors/sensor_type.hpp"

WaterLevelClientMock::WaterLevelClientMock(
    std::shared_ptr<MqttClient> mqttClient)
    : SensorClient(std::move(mqttClient), SensorType::WaterLevel) {}

float WaterLevelClientMock::readValue() { return 87.5f; }
