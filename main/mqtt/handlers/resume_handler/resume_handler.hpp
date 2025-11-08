#pragma once

#include <array>
#include <memory>
#include <mqtt/handlers/device_control_handler/device_control_handler.hpp>

#include "mqtt/mqtt_event_handler.hpp"
#include "polling_manager/sensor_polling_manager/sensor_polling_manager.hpp"

class ResumeHandler final : public MqttEventHandler {
   public:
    ResumeHandler(std::shared_ptr<MqttClient> mqtt,
                  std::array<std::shared_ptr<PollingManager>, 4> sensors,
                  std::shared_ptr<DeviceControlHandler> deviceHandler);
    void operate(const std::string& topic, const std::string& payload) override;

   private:
    std::shared_ptr<MqttClient> mqtt;
    std::array<std::shared_ptr<PollingManager>, 4> sensors;
    std::shared_ptr<DeviceControlHandler> deviceHandler;
};