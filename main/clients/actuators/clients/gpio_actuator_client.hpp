#pragma once

#include <driver/gpio.h>

#include "clients/actuators/clients/actuator_client.hpp"

class GpioActuatorClient final : public ActuatorClient {
   public:
    GpioActuatorClient(std::shared_ptr<MqttClient> mqttClient,
                       const ActuatorType type, const int gardenId,
                       gpio_num_t gpioNum);

   protected:
    bool performTurnOn() override;
    bool performTurnOff() override;

   private:
    gpio_num_t gpio;
};
