#pragma once

#include "clients/sensors/clients/sensor_client.hpp"

class ThermometerClientMock final : public SensorClient {
   public:
    ThermometerClientMock(std::shared_ptr<MqttClient> mqttClient,
                          const int gardenId);

   protected:
    float readValue() override;
};
