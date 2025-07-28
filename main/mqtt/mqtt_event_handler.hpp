#pragma once
#include <string>

class MqttEventHandler {
   public:
    virtual ~MqttEventHandler() = default;
    virtual void operate(const std::string& topic,
                         const std::string& payload) = 0;
};
