#pragma once

#include <memory>
#include <string>

#include "clients/sensors/clients/sensor_client.hpp"
#include "clients/sensors/sensor_type.hpp"
#include "mqtt/mqtt_client/mqtt_client_impl.hpp"

class SensorFactory {
   public:
    virtual std::shared_ptr<SensorClient> create(
        const SensorType type, std::shared_ptr<MqttClient> mqtt) const = 0;
};

class MockSensorFactory final : public SensorFactory {
   public:
    std::shared_ptr<SensorClient> create(
        const SensorType type, std::shared_ptr<MqttClient> mqtt) const override;
};

class AdcSensorFactory final : public SensorFactory {
   public:
    std::shared_ptr<SensorClient> create(
        const SensorType type, std::shared_ptr<MqttClient> mqtt) const override;
};