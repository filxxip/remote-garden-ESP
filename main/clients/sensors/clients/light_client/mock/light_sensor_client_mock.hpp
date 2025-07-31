#pragma once

#include "clients/sensors/clients/sensor_client.hpp"

class LightSensorClientMock final : public SensorClient {
   public:
    LightSensorClientMock(std::shared_ptr<MqttClient> mqttClient,
                          const int gardenId);

   protected:
    float readValue() override;
};
