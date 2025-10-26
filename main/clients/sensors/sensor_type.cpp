#include "sensor_type.hpp"

#include "utils/enum/enum_mapper.hpp"
#include "utils/enum/enum_utils.hpp"

namespace {
const std::unordered_map<SensorType, std::string> sensorToStr{
    {SensorType::Light, "light"},
    {SensorType::AirHumidity, "air_humidity"},
    {SensorType::SoilMoisture, "soil_moisture"},
    {SensorType::AirTemperature, "air_temperature"},
    {SensorType::SignalStrength, "signal_strenght"},
    {SensorType::WaterLevel, "water_level"},
    {SensorType::Camera, "camera"},
};

const std::unordered_map<std::string, SensorType> strToSensor =
    reverse_enum_map(sensorToStr);
}  // namespace

template <>
const std::unordered_map<SensorType, std::string>&
getEnumToStrMap<SensorType>() {
    return sensorToStr;
}

template <>
const std::unordered_map<std::string, SensorType>&
getStrToEnumMap<SensorType>() {
    return strToSensor;
}
