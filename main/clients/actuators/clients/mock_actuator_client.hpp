#pragma once

#include "clients/actuators/clients/actuator_client.hpp"

class MockActuatorClient final : public ActuatorClient {
   public:
    using ActuatorClient::ActuatorClient;

   protected:
    bool performTurnOn() override;
    bool performTurnOff() override;
};
