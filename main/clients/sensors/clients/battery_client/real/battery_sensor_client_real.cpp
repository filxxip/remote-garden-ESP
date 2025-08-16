#include "battery_sensor_client_real.hpp"

BatterySensorClientReal::BatterySensorClientReal(
    std::shared_ptr<MqttClient> mqttClient, adc_unit_t adcUnit,
    adc_channel_t adcChannel)
    : AdcSensorClient(std::move(mqttClient), SensorType::Battery, adcUnit,
                      adcChannel) {}

float BatterySensorClientReal::convertRawToValue(int raw) const {
    float voltage = (static_cast<float>(raw) / 4095.0f) * 3.3f;
    float realVoltage = voltage * 2.0f;
    if (realVoltage >= 4.2f) return 100.0f;
    if (realVoltage <= 3.0f) return 0.0f;
    return (realVoltage - 3.0f) / (4.2f - 3.0f) * 100.0f;
}
