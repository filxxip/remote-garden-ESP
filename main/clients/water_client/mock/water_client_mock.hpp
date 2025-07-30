#pragma once

#include "clients/actuator_client.hpp"

class WaterClientMock : public ActuatorClient {
   public:
    WaterClientMock(std::shared_ptr<MqttClient> mqttClient, int gardenId);

   protected:
    bool performTurnOn() override;
    bool performTurnOff() override;
};
