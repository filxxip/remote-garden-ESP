#pragma once

#include "clients/sensors/clients/adc_sensor_client.hpp"

class LightSensorClientReal final : public AdcSensorClient {
   public:
    LightSensorClientReal(std::shared_ptr<MqttClient> mqttClient,
                          const adc_unit_t adcUnit,
                          const adc_channel_t adcChannel);

   protected:
    float convertRawToValue(const int raw) const override;
};
