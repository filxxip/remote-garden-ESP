#pragma once
#include "../../sensor_client.hpp"
#include "driver/adc.h"

class SoilMoistureClientReal : public SensorClient {
   public:
    SoilMoistureClientReal(std::shared_ptr<MqttClient> mqttClient,
                           adc2_channel_t channel);

    bool initializeImpl() override;

   protected:
    float readValue() override;

   private:
    adc2_channel_t adcChannel;
};
