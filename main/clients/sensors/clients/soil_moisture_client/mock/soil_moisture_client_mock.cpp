#include "soil_moisture_client_mock.hpp"

#include "clients/sensors/sensor_type.hpp"

SoilMoistureClientMock::SoilMoistureClientMock(
    std::shared_ptr<MqttClient> mqttClient)
    : SensorClient(std::move(mqttClient), SensorType::SoilMoisture) {}

float SoilMoistureClientMock::readValue() { return 42.0f; }
