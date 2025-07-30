#pragma once

#include "clients/sensor_client.hpp"

class LightSensorClientMock : public SensorClient {
   public:
    LightSensorClientMock(std::shared_ptr<MqttClient> mqttClient, int gardenId);

   protected:
    float readValue() override;
};
