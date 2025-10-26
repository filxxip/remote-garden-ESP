#pragma once

#include "../../sensor_client.hpp"
#include "driver/i2c.h"

class LightSensorClientReal : public SensorClient {
   public:
    LightSensorClientReal(std::shared_ptr<MqttClient> mqttClient,
                          i2c_port_t i2cPort, gpio_num_t sdaPin,
                          gpio_num_t sclPin);

    ~LightSensorClientReal();

   protected:
    float readValue() override;

   private:
    i2c_port_t port;
    uint8_t address;
    bool i2c_initialized;

    esp_err_t writeCommand(uint8_t cmd);
    esp_err_t readLightLevel(uint16_t* lux);
};