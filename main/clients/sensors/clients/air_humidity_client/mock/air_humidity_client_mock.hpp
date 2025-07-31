#pragma once

#include "clients/sensors/clients/sensor_client.hpp"

class AirHumidityClientMock final : public SensorClient {
   public:
    AirHumidityClientMock(std::shared_ptr<MqttClient> mqttClient,
                          const int gardenId);

   protected:
    float readValue() override;
};
