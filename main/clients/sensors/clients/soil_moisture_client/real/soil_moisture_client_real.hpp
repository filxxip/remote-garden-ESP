#pragma once

#include "clients/sensors/clients/adc_sensor_client.hpp"

class SoilMoistureClientReal final : public AdcSensorClient {
   public:
    SoilMoistureClientReal(std::shared_ptr<MqttClient> mqttClient, int gardenId,
                           const adc_unit_t adcUnit,
                           const adc_channel_t adcChannel);

   protected:
    float convertRawToValue(const int raw) const override;
};
