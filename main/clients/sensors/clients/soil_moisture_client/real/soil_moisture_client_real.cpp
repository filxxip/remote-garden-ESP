#include "soil_moisture_client_real.hpp"

#include "clients/sensors/sensor_type.hpp"

SoilMoistureClientReal::SoilMoistureClientReal(
    std::shared_ptr<MqttClient> mqttClient, const int gardenId,
    const adc_unit_t adcUnit, const adc_channel_t adcChannel)
    : AdcSensorClient(std::move(mqttClient), SensorType::SoilMoisture, gardenId,
                      adcUnit, adcChannel) {}

float SoilMoistureClientReal::convertRawToValue(const int raw) const {
    return (static_cast<float>(raw) / 4095.0f) * 100.0f;
}
