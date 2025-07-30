#pragma once

#include "clients/actuator_client.hpp"

class HeatingMatClientReal : public ActuatorClient {
   public:
    HeatingMatClientReal(std::shared_ptr<MqttClient> mqttClient, int gardenId);

   protected:
    bool performTurnOn() override;
    bool performTurnOff() override;
};
