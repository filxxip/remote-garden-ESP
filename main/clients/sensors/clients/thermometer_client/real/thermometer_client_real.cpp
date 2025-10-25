#include "thermometer_client_real.hpp"

#include "esp_log.h"

namespace {
const char* TAG = "ThermometerClientReal";
}

ThermometerClientReal::ThermometerClientReal(
    std::shared_ptr<MqttClient> mqttClient, std::shared_ptr<Dht11Reader> dht)
    : SensorClient(std::move(mqttClient), SensorType::AirTemperature),
      dhtReader(std::move(dht)) {}

float ThermometerClientReal::readValue() {
    dhtReader->update();
    float value = dhtReader->getTemperature();
    ESP_LOGI(TAG, "Temperature reading: %.2f°C", value);
    return value;
}
