#pragma once

#include <string>

#include "mqtt/mqtt_client/mqtt_client_impl.hpp"

class ActuatorClient {
   public:
    ActuatorClient(std::shared_ptr<MqttClient> mqttClient,
                   const std::string& deviceName, const int gardenId);

    virtual ~ActuatorClient() = default;

    void turnOn();
    void turnOff();

   protected:
    virtual bool performTurnOn() = 0;
    virtual bool performTurnOff() = 0;

    void confirmOn(bool success);
    void confirmOff(bool success);

   private:
    std::shared_ptr<MqttClient> mqtt;
    std::string deviceName;
    std::string confirmationTopic;
};
