#include "signal_strength_client_mock.hpp"

#include "clients/sensors/sensor_type.hpp"

SignalStrengthClientMock::SignalStrengthClientMock(
    std::shared_ptr<MqttClient> mqttClient)
    : SensorClient(std::move(mqttClient), SensorType::SignalStrength) {}

float SignalStrengthClientMock::readValue() { return -60.0f; }
