#include "actuator_client.hpp"

#include <string>

ActuatorClient::ActuatorClient(std::shared_ptr<MqttClient> mqttClient,
                               const std::string& deviceName,
                               const int gardenId)
    : mqtt(std::move(mqttClient)),
      deviceName(deviceName),
      confirmationTopic("device/" + std::to_string(gardenId) + "/confirm") {}

void ActuatorClient::turnOn() { confirmOn(performTurnOn()); }

void ActuatorClient::turnOff() { confirmOff(performTurnOff()); }

void ActuatorClient::confirmOn(bool success) {
    std::string payload =
        std::string("{\"device\":\"") + deviceName +
        "\",\"action\":\"on\",\"status\":" + (success ? "true" : "false") + "}";

    mqtt->publish(confirmationTopic, payload);
}

void ActuatorClient::confirmOff(bool success) {
    std::string payload =
        std::string("{\"device\":\"") + deviceName +
        "\",\"action\":\"off\",\"status\":" + (success ? "true" : "false") +
        "}";

    mqtt->publish(confirmationTopic, payload);
}
