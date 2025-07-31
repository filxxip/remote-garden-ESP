#include "actuator_factory.hpp"

#include "clients/actuators/clients/mock_actuator_client.hpp"

namespace {
const std::unordered_map<ActuatorType, gpio_num_t> gpioMap = {
    {ActuatorType::Fan, GPIO_NUM_17},
    {ActuatorType::Water, GPIO_NUM_5},
    {ActuatorType::Atomize, GPIO_NUM_16},
    {ActuatorType::HeatingMat, GPIO_NUM_18},
};
}

std::unique_ptr<ActuatorClient> MockActuatorFactory::create(
    const ActuatorType type, std::shared_ptr<MqttClient> mqtt,
    int gardenId) const {
    return std::make_unique<MockActuatorClient>(std::move(mqtt), type,
                                                gardenId);
}

std::unique_ptr<ActuatorClient> GpioActuatorFactory::create(
    const ActuatorType type, std::shared_ptr<MqttClient> mqtt,
    int gardenId) const {
    auto it = gpioMap.find(type);
    if (it == gpioMap.end()) {
        return nullptr;
    };

    return std::make_unique<GpioActuatorClient>(std::move(mqtt), type, gardenId,
                                                it->second);
}
