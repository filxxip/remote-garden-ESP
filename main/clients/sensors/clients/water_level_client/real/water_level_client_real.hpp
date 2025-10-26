#pragma once
#include <array>

#include "../../sensor_client.hpp"
#include "driver/i2c.h"

class WaterLevelClientReal : public SensorClient {
   public:
    WaterLevelClientReal(std::shared_ptr<MqttClient> mqttClient,
                         i2c_port_t i2cPort);

   protected:
    float readValue() override;

   private:
    i2c_port_t port;
};