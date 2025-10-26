#pragma once

#include "../../dht_11_client.hpp"
#include "../../sensor_client.hpp"

class ThermometerClientReal final : public SensorClient {
   public:
    ThermometerClientReal(std::shared_ptr<MqttClient> mqttClient,
                          std::shared_ptr<Dht11Reader> dht);

   protected:
    float readValue() override;

   private:
    std::shared_ptr<Dht11Reader> dhtReader;
};
