#pragma once

#include "clients/actuator_client.hpp"

class WaterClientReal : public ActuatorClient {
   public:
    WaterClientReal(std::shared_ptr<MqttClient> mqttClient, int gardenId);

   protected:
    bool performTurnOn() override;
    bool performTurnOff() override;
};
