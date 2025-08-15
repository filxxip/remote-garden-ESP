#include "light_sensor_client_real.hpp"

#include "clients/sensors/sensor_type.hpp"

LightSensorClientReal::LightSensorClientReal(
    std::shared_ptr<MqttClient> mqttClient, const adc_unit_t adcUnit,
    const adc_channel_t adcChannel)
    : AdcSensorClient(std::move(mqttClient), SensorType::Light, adcUnit,
                      adcChannel) {}

float LightSensorClientReal::convertRawToValue(const int raw) const {
    return (static_cast<float>(raw) / 4095.0f) * 3.3f;
}
