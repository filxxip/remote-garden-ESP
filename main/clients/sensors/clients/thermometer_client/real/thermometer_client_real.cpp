#include "thermometer_client_real.hpp"

#include "clients/sensors/sensor_type.hpp"

ThermometerClientReal::ThermometerClientReal(
    std::shared_ptr<MqttClient> mqttClient, const int gardenId,
    const adc_unit_t adcUnit, const adc_channel_t adcChannel)
    : AdcSensorClient(std::move(mqttClient), SensorType::AirTemperature,
                      gardenId, adcUnit, adcChannel) {}

float ThermometerClientReal::convertRawToValue(const int raw) const {
    float voltage = (static_cast<float>(raw) / 4095.0f) * 3.3f;
    return (voltage - 0.5f) * 100.0f;  // TMP36: 10 mV/°C offset by 0.5 V
}
