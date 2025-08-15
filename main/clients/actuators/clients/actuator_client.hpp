#pragma once

#include <string>

#include "clients/actuators/actuator_type.hpp"
#include "mqtt/mqtt_client/mqtt_client_impl.hpp"

class ActuatorClient {
   public:
    ActuatorClient(std::shared_ptr<MqttClient> mqttClient,
                   const ActuatorType type);

    virtual ~ActuatorClient() = default;

    void turnOn();
    void turnOff();

   protected:
    virtual bool performTurnOn() = 0;
    virtual bool performTurnOff() = 0;

    void confirmOn(bool success);
    void confirmOff(bool success);

    ActuatorType type;

   private:
    std::shared_ptr<MqttClient> mqtt;
    std::string confirmationTopic;
};
