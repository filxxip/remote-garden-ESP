#pragma once

#include "clients/sensors/clients/sensor_client.hpp"

class SoilMoistureClientMock final : public SensorClient {
   public:
    SoilMoistureClientMock(std::shared_ptr<MqttClient> mqttClient);

   protected:
    float readValue() override;
};
