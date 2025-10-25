#pragma once

#include "clients/sensors/clients/sensor_client.hpp"

class WaterLevelClientMock final : public SensorClient {
   public:
    WaterLevelClientMock(std::shared_ptr<MqttClient> mqttClient);

   protected:
    float readValue() override;
};
