#include "clients/sensors/sensor_type.hpp"
#include "soil_moisture_client_mock.hpp"

SoilMoistureClientMock::SoilMoistureClientMock(
    std::shared_ptr<MqttClient> mqttClient, const int gardenId)
    : SensorClient(std::move(mqttClient), SensorType::SoilMoisture, gardenId) {}

float SoilMoistureClientMock::readValue() { return 42.0f; }
