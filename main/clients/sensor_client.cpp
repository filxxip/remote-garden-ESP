#include "sensor_client.hpp"

#include <string>

#include "esp_log.h"

SensorClient::SensorClient(std::shared_ptr<MqttClient> mqttClient,
                           std::string sensorName, int gardenId)
    : mqtt(std::move(mqttClient)),
      topic("device/" + std::to_string(gardenId) + "/sensor/" + sensorName),
      sensorName(std::move(sensorName)) {}

void SensorClient::publishReading() {
    float value = readValue();
    ESP_LOGI("SensorClient", "Read value from sensor '%s': %.2f",
             sensorName.c_str(), value);
    std::string payload = "{\"sensor\":\"" + sensorName +
                          "\",\"value\":" + std::to_string(value) + "}";
    mqtt->publish(topic, payload);
}
