#pragma once

#include "clients/actuator_client.hpp"

class HeatingMatClientMock : public ActuatorClient {
   public:
    HeatingMatClientMock(std::shared_ptr<MqttClient> mqttClient, int gardenId);

   protected:
    bool performTurnOn() override;
    bool performTurnOff() override;
};