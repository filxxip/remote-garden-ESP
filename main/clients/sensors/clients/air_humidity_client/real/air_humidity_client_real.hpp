#pragma once

#include "../../dht_11_client.hpp"
#include "../../sensor_client.hpp"

class AirHumidityClientReal final : public SensorClient {
   public:
    AirHumidityClientReal(std::shared_ptr<MqttClient> mqttClient,
                          std::shared_ptr<Dht11Reader> dht);

   protected:
    float readValue() override;

   private:
    std::shared_ptr<Dht11Reader> dhtReader;
};
