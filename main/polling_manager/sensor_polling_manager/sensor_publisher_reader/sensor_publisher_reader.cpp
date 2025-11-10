#include "sensor_publisher_reader.hpp"

#include <esp_log.h>

namespace {
constexpr auto TAG = "SensorPublisherReader";
}

void SensorPublisherReader::action(
    const std::shared_ptr<SensorClient>& sensor) const {
    if (sensor->isInitialized()) {
        sensor->storeReading();
        sensor->flushCache();
    } else {
        ESP_LOGW(TAG, "Sensor is not yet initialized");
    }
}