#pragma once

#include "clients/sensors/clients/sensor_client.hpp"
#include "wifi_manager/wifi_manager.hpp"

class SignalStrengthClientReal final : public SensorClient {
   public:
    SignalStrengthClientReal(std::shared_ptr<MqttClient> mqttClient,
                             std::shared_ptr<WifiManager> wifi);

   protected:
    float readValue() override;

   private:
    std::shared_ptr<WifiManager> wifi;
};
