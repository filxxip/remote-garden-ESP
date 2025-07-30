#pragma once

#include "clients/actuator_client.hpp"

class FanClientReal : public ActuatorClient {
   public:
    FanClientReal(std::shared_ptr<MqttClient> mqttClient, int gardenId);

   protected:
    bool performTurnOn() override;
    bool performTurnOff() override;
};
