#pragma once

#include <memory>

#include "mqtt/heavy_mqtt_event_handler.hpp"
#include "mqtt/mqtt_client/mqtt_client_impl.hpp"
#include "wifi_manager/wifi_manager.hpp"

class ResetHandler final : public HeavyMqttEventHandler {
   public:
    ResetHandler(std::shared_ptr<WifiManager> wifi,
                 std::shared_ptr<MqttClient> mqtt);
    void handleHeavy(const std::string& topic,
                     const std::string& payload) override;

   private:
    std::shared_ptr<WifiManager> wifi;
    std::shared_ptr<MqttClient> mqtt;
};