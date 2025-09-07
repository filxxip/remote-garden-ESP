#pragma once

#include <memory>

#include "mqtt/mqtt_event_handler.hpp"
#include "polling_manager/sensor_polling_manager/sensor_polling_manager.hpp"

class ResumeHandler final : public MqttEventHandler {
   public:
    ResumeHandler(std::array<std::shared_ptr<PollingManager>, 2> sensors);
    void operate(const std::string& topic, const std::string& payload) override;

   private:
    std::array<std::shared_ptr<PollingManager>, 2> sensors;
};