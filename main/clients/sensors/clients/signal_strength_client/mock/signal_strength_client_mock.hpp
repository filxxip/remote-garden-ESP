#pragma once

#include "clients/sensors/clients/sensor_client.hpp"

class SignalStrengthClientMock final : public SensorClient {
   public:
    SignalStrengthClientMock(std::shared_ptr<MqttClient> mqttClient);

   protected:
    float readValue() override;
};
