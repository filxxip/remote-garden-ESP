#pragma once

#include "clients/sensors/clients/adc_sensor_client.hpp"

class ThermometerClientReal final : public AdcSensorClient {
   public:
    ThermometerClientReal(std::shared_ptr<MqttClient> mqttClient,
                          const adc_unit_t adcUnit,
                          const adc_channel_t adcChannel);

   protected:
    float convertRawToValue(const int raw) const override;
};
