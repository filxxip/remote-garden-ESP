#pragma once

#include "mqtt/mqtt_event_handler.hpp"

class MyHandler : public MqttEventHandler {
   public:
    void operate(const std::string& topic, const std::string& payload) override;
};