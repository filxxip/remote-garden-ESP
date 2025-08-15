#pragma once

#include "clients/sensors/clients/sensor_client.hpp"

class BatterySensorClientMock final : public SensorClient {
   public:
    BatterySensorClientMock(std::shared_ptr<MqttClient> mqttClient);

   protected:
    float readValue() override;
};
