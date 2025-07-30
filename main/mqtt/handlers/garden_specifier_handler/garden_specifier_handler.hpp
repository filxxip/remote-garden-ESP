#pragma once

#include <functional>
#include <optional>

#include "mqtt/mqtt_event_handler.hpp"

class GardenSpecifierHandler : public MqttEventHandler {
   public:
    void operate(const std::string& topic, const std::string& payload) override;

    void setCallback(std::function<void(const std::optional<int>&, int)> cb);

   private:
    std::function<void(const std::optional<int>&, int)> callback;
    std::optional<int> garden;
};