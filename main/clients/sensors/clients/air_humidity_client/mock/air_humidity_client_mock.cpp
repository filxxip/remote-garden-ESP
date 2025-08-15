#include "air_humidity_client_mock.hpp"

#include "clients/sensors/clients/adc_sensor_client.hpp"

AirHumidityClientMock::AirHumidityClientMock(
    std::shared_ptr<MqttClient> mqttClient)
    : SensorClient(std::move(mqttClient), SensorType::AirHumidity) {}

float AirHumidityClientMock::readValue() { return 55.5f; }
