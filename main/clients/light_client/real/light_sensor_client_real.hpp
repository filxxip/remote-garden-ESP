#pragma once

#include "clients/sensor_client.hpp"
#include "esp_adc/adc_oneshot.h"

class LightSensorClientReal : public SensorClient {
   public:
    LightSensorClientReal(std::shared_ptr<MqttClient> mqttClient, int gardenId,
                          adc_unit_t adcUnit, adc_channel_t adcChannel);

    ~LightSensorClientReal();

   protected:
    float readValue() override;

   private:
    adc_oneshot_unit_handle_t adcHandle;
    adc_channel_t channel;
};
