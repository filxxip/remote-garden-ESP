#pragma once

#include "clients/sensors/clients/adc_sensor_client.hpp"

class BatterySensorClientReal final : public AdcSensorClient {
   public:
    BatterySensorClientReal(std::shared_ptr<MqttClient> mqttClient,
                            adc_unit_t adcUnit, adc_channel_t adcChannel);

   protected:
    float convertRawToValue(int raw) const override;
};
