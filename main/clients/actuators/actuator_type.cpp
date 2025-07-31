#include "actuator_type.hpp"

#include "utils/enum/enum_mapper.hpp"
#include "utils/enum/enum_utils.hpp"

namespace {
const std::unordered_map<ActuatorType, std::string> actuatorToStr{
    {ActuatorType::Water, "water"},
    {ActuatorType::Fan, "fan"},
    {ActuatorType::Atomize, "atomize"},
    {ActuatorType::HeatingMat, "heating_mat"},
};

const std::unordered_map<std::string, ActuatorType> strToActuator =
    reverse_enum_map(actuatorToStr);
}  // namespace

template <>
const std::unordered_map<ActuatorType, std::string>&
getEnumToStrMap<ActuatorType>() {
    return actuatorToStr;
}

template <>
const std::unordered_map<std::string, ActuatorType>&
getStrToEnumMap<ActuatorType>() {
    return strToActuator;
}
