#include "sensor_client.hpp"

#include <numeric>

#include "esp_log.h"
#include "utils/enum/enum_mapper.hpp"

namespace {
constexpr const char* TAG = "SensorClient";
}

SensorClient::SensorClient(std::shared_ptr<MqttClient> mqttClient,
                           const SensorType type, const int gardenId)
    : sensorType(type),
      mqtt(std::move(mqttClient)),
      topic("device/" + std::to_string(gardenId) + "/sensor/" +
            enumToString(type)) {}

void SensorClient::storeReading() {
    auto value = readValue();
    cache.push_back(value);
    ESP_LOGI(TAG, "Stored reading for '%s': %.2f",
             enumToString(sensorType).c_str(), value);
}

void SensorClient::flushCache() {
    if (cache.empty()) {
        ESP_LOGW(TAG, "No readings to flush for sensor '%s'",
                 enumToString(sensorType).c_str());
        return;
    }

    auto values = std::accumulate(std::next(cache.begin()), cache.end(),
                                  std::to_string(cache.front()),
                                  [](const auto& acc, const auto val) {
                                      return acc + "," + std::to_string(val);
                                  });

    auto payload = "{\"sensor\":\"" + enumToString(sensorType) +
                   "\",\"values\":[" + values + "]}";

    if (mqtt->publish(topic, payload)) {
        ESP_LOGI(TAG, "Published %zu cached readings for '%s'", cache.size(),
                 enumToString(sensorType).c_str());
        cache.clear();
    } else {
        ESP_LOGW(TAG,
                 "Failed to publish cached readings for '%s', keeping them",
                 enumToString(sensorType).c_str());
    }
}
