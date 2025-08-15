#pragma once

#include "clients/sensors/clients/sensor_client.hpp"

class ThermometerClientMock final : public SensorClient {
   public:
    ThermometerClientMock(std::shared_ptr<MqttClient> mqttClient);

   protected:
    float readValue() override;
};
