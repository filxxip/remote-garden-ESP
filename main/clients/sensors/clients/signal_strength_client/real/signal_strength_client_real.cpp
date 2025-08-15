#include "signal_strength_client_real.hpp"

#include "clients/sensors/sensor_type.hpp"
#include "esp_log.h"
#include "esp_wifi.h"

namespace {
constexpr const char* TAG = "SignalStrengthClientReal";
}

SignalStrengthClientReal::SignalStrengthClientReal(
    std::shared_ptr<MqttClient> mqttClient, std::shared_ptr<WifiManager> wifi, )
    : SensorClient(std::move(mqttClient), SensorType::SignalStrength),
      wifi(std::move(wifi)) {}

float SignalStrengthClientReal::readValue() { return wifi->getRssi(); }
