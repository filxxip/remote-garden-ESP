#include "air_humidity_client_mock.hpp"

#include "clients/sensors/clients/adc_sensor_client.hpp"

AirHumidityClientMock::AirHumidityClientMock(
    std::shared_ptr<MqttClient> mqttClient, const int gardenId)
    : SensorClient(std::move(mqttClient), SensorType::AirHumidity, gardenId) {}

float AirHumidityClientMock::readValue() { return 55.5f; }
