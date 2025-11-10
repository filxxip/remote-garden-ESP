#pragma once

#include <esp_adc/adc_oneshot.h>

#include "clients/sensors/clients/sensor_client.hpp"
#include "clients/sensors/sensor_type.hpp"

class AdcSensorClient : public SensorClient {
   public:
    AdcSensorClient(std::shared_ptr<MqttClient> mqttClient,
                    const SensorType type, const adc_unit_t adcUnit,
                    const adc_channel_t adcChannel);
    ~AdcSensorClient() override;

    bool initializeImpl() override;

   protected:
    virtual float convertRawToValue(int raw) const = 0;
    float readValue() override;

   private:
    adc_oneshot_unit_handle_t adcHandle;
    adc_channel_t channel;
    const adc_unit_t adcUnit;
};
