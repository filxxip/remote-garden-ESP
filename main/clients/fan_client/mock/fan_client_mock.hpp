#pragma once

#include "clients/actuator_client.hpp"

class FanClientMock : public ActuatorClient {
   public:
    FanClientMock(std::shared_ptr<MqttClient> mqttClient, int gardenId);

   protected:
    bool performTurnOn() override;
    bool performTurnOff() override;
};