#include "sensor_publisher.hpp"

#include <esp_log.h>

namespace {
constexpr auto TAG = "SensorPublisher";
}

void SensorPublisher::action(
    const std::shared_ptr<SensorClient>& sensor) const {
    if (sensor->isInitialized()) {
        sensor->flushCache();
    } else {
        ESP_LOGW(TAG, "Sensor is not yet initialized");
    }
}