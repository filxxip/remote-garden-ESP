#pragma once

#include "clients/sensors/clients/adc_sensor_client.hpp"

class AirHumidityClientReal final : public AdcSensorClient {
   public:
    AirHumidityClientReal(std::shared_ptr<MqttClient> mqttClient,
                          const adc_unit_t adcUnit,
                          const adc_channel_t adcChannel);

   protected:
    float convertRawToValue(int raw) const override;
};
