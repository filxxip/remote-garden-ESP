#include "air_humidity_client_real.hpp"

#include "esp_log.h"

namespace {
const char* TAG = "AirHumidityClientReal";
}

AirHumidityClientReal::AirHumidityClientReal(
    std::shared_ptr<MqttClient> mqttClient, std::shared_ptr<Dht11Reader> dht)
    : SensorClient(std::move(mqttClient), SensorType::AirHumidity),
      dhtReader(std::move(dht)) {}

float AirHumidityClientReal::readValue() {
    dhtReader->update();
    float value = dhtReader->getHumidity();
    ESP_LOGI(TAG, "Humidity reading: %.2f%%", value);
    return value;
}
