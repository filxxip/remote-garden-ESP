#include "battery_sensor_client_mock.hpp"

#include "clients/sensors/sensor_type.hpp"

BatterySensorClientMock::BatterySensorClientMock(
    std::shared_ptr<MqttClient> mqttClient)
    : SensorClient(std::move(mqttClient), SensorType::Battery) {}

float BatterySensorClientMock::readValue() { return 87.5f; }
