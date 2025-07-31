#include "air_humidity_client_real.hpp"

#include "clients/sensors/sensor_type.hpp"

AirHumidityClientReal::AirHumidityClientReal(
    std::shared_ptr<MqttClient> mqttClient, const int gardenId,
    const adc_unit_t adcUnit, const adc_channel_t adcChannel)
    : AdcSensorClient(std::move(mqttClient), SensorType::AirHumidity, gardenId,
                      adcUnit, adcChannel) {}

float AirHumidityClientReal::convertRawToValue(const int raw) const {
    auto voltage = (static_cast<float>(raw) / 4095.0f) * 3.3f;
    auto humidity = (voltage / 3.3f) * 100.0f;
    return humidity;
}
