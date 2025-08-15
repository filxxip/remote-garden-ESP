#pragma once

#include "mqtt/mqtt_event_handler.hpp"

class ResetHandler : public MqttEventHandler {
   public:
    ResetHandler(std::shared_ptr<WifiManager> wifi,
                 std::shared_ptr<MqttClient> mqtt);
    void operate(const std::string& topic, const std::string& payload) override;

   private:
    std::shared_ptr<WifiManager> wifi;
    std::shared_ptr<MqttClient> mqtt;
};