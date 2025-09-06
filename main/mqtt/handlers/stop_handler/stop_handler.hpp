#pragma once

#include <memory>

#include "mqtt/mqtt_client/mqtt_client_impl.hpp"
#include "mqtt/mqtt_event_handler.hpp"
#include "sensor_polling_manager/sensor_polling_manager.hpp"
#include "wifi_manager/wifi_manager.hpp"

class StopHandler : public MqttEventHandler {
   public:
    StopHandler(std::shared_ptr<MqttClient> mqtt,
                std::array<std::shared_ptr<SensorPollingManager>, 2> sensors);
    void operate(const std::string& topic, const std::string& payload) override;

   private:
    std::shared_ptr<MqttClient> mqtt;
    std::array<std::shared_ptr<SensorPollingManager>, 2> sensors;
};