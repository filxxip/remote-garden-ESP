#include "actuator_client.hpp"

#include "utils/enum/enum_mapper.hpp"

ActuatorClient::ActuatorClient(std::shared_ptr<MqttClient> mqttClient,
                               const ActuatorType type)
    : type(type),
      mqtt(std::move(mqttClient)),
      confirmationTopic("device/confirm") {}

void ActuatorClient::turnOn() { confirmOn(performTurnOn()); }

void ActuatorClient::turnOff() { confirmOff(performTurnOff()); }

void ActuatorClient::confirmOn(bool success) {
    auto payload =
        "{\"device\":\"" + enumToString(type) +
        "\",\"action\":\"on\",\"status\":" + (success ? "true" : "false") + "}";
    mqtt->publish(confirmationTopic, payload);
}

void ActuatorClient::confirmOff(bool success) {
    auto payload =
        "{\"device\":\"" + enumToString(type) +
        "\",\"action\":\"off\",\"status\":" + (success ? "true" : "false") +
        "}";
    mqtt->publish(confirmationTopic, payload);
}