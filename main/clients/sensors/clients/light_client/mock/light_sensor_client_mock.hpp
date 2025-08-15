#pragma once

#include "clients/sensors/clients/sensor_client.hpp"

class LightSensorClientMock final : public SensorClient {
   public:
    LightSensorClientMock(std::shared_ptr<MqttClient> mqttClient);

   protected:
    float readValue() override;
};
