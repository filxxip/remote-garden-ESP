#include "thermometer_client_mock.hpp"

#include "clients/sensors/sensor_type.hpp"

ThermometerClientMock::ThermometerClientMock(
    std::shared_ptr<MqttClient> mqttClient)
    : SensorClient(std::move(mqttClient), SensorType::AirTemperature) {}

float ThermometerClientMock::readValue() { return 22.7f; }
