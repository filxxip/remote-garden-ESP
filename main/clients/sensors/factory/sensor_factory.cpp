#include "sensor_factory.hpp"

#include <unordered_map>

#include "clients/sensors/clients/air_humidity_client/mock/air_humidity_client_mock.hpp"
#include "clients/sensors/clients/air_humidity_client/real/air_humidity_client_real.hpp"
#include "clients/sensors/clients/light_client/mock/light_sensor_client_mock.hpp"
#include "clients/sensors/clients/light_client/real/light_sensor_client_real.hpp"
#include "clients/sensors/clients/soil_moisture_client/mock/soil_moisture_client_mock.hpp"
#include "clients/sensors/clients/soil_moisture_client/real/soil_moisture_client_real.hpp"
#include "clients/sensors/clients/thermometer_client/mock/thermometer_client_mock.hpp"
#include "clients/sensors/clients/thermometer_client/real/thermometer_client_real.hpp"

namespace {
using UnitChan = std::pair<adc_unit_t, adc_channel_t>;

const std::unordered_map<SensorType, UnitChan> adcConfig = {
    {SensorType::AirHumidity, {ADC_UNIT_1, ADC_CHANNEL_3}},
    {SensorType::Light, {ADC_UNIT_1, ADC_CHANNEL_4}},
    {SensorType::SoilMoisture, {ADC_UNIT_1, ADC_CHANNEL_5}},
    {SensorType::AirTemperature, {ADC_UNIT_1, ADC_CHANNEL_6}},
};
}  // namespace

std::shared_ptr<SensorClient> MockSensorFactory::create(
    const SensorType type, std::shared_ptr<MqttClient> mqtt,
    const int gardenId) const {
    switch (type) {
        case SensorType::AirHumidity:
            return std::make_shared<AirHumidityClientMock>(std::move(mqtt),
                                                           gardenId);
        case SensorType::Light:
            return std::make_shared<LightSensorClientMock>(std::move(mqtt),
                                                           gardenId);
        case SensorType::SoilMoisture:
            return std::make_shared<SoilMoistureClientMock>(std::move(mqtt),
                                                            gardenId);
        case SensorType::AirTemperature:
            return std::make_shared<ThermometerClientMock>(std::move(mqtt),
                                                           gardenId);
        default:
            return nullptr;
    }
}

std::shared_ptr<SensorClient> AdcSensorFactory::create(
    const SensorType type, std::shared_ptr<MqttClient> mqtt,
    const int gardenId) const {
    auto it = adcConfig.find(type);
    if (it == adcConfig.end()) return nullptr;

    const auto [unit, channel] = it->second;

    switch (type) {
        case SensorType::AirHumidity:
            return std::make_shared<AirHumidityClientReal>(
                std::move(mqtt), gardenId, unit, channel);
        case SensorType::Light:
            return std::make_shared<LightSensorClientReal>(
                std::move(mqtt), gardenId, unit, channel);
        case SensorType::SoilMoisture:
            return std::make_shared<SoilMoistureClientReal>(
                std::move(mqtt), gardenId, unit, channel);
        case SensorType::AirTemperature:
            return std::make_shared<ThermometerClientReal>(
                std::move(mqtt), gardenId, unit, channel);
        default:
            return nullptr;
    }
}
