#pragma once

#include <memory>
#include <string>

#include "clients/actuators/actuator_type.hpp"
#include "clients/actuators/clients/gpio_actuator_client.hpp"

class ActuatorFactory {
   public:
    virtual std::unique_ptr<ActuatorClient> create(
        const ActuatorType type, std::shared_ptr<MqttClient> mqtt) const = 0;
};

class GpioActuatorFactory final : public ActuatorFactory {
   public:
    std::unique_ptr<ActuatorClient> create(
        const ActuatorType type,
        std::shared_ptr<MqttClient> mqtt) const override;
};

class MockActuatorFactory final : public ActuatorFactory {
   public:
    std::unique_ptr<ActuatorClient> create(
        const ActuatorType type,
        std::shared_ptr<MqttClient> mqtt) const override;
};