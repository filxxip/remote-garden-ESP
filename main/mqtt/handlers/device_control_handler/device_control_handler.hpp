#pragma once

#include "clients/actuator_client.hpp"
#include "mqtt/mqtt_event_handler.hpp"

class DeviceControlHandler : public MqttEventHandler {
   public:
    DeviceControlHandler(std::unique_ptr<ActuatorClient> atomize,
                         std::unique_ptr<ActuatorClient> fan,
                         std::unique_ptr<ActuatorClient> heatingMat,
                         std::unique_ptr<ActuatorClient> water);
    void operate(const std::string& topic, const std::string& payload) override;

   private:
    std::unique_ptr<ActuatorClient> atomize;
    std::unique_ptr<ActuatorClient> fan;
    std::unique_ptr<ActuatorClient> heatingMat;
    std::unique_ptr<ActuatorClient> water;
};