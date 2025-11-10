#include "sensor_reader.hpp"

#include <esp_log.h>

namespace {
constexpr auto TAG = "SensorReader";
}

void SensorReader::action(const std::shared_ptr<SensorClient>& sensor) const {
    if (sensor->isInitialized()) {
        sensor->storeReading();
    } else {
        ESP_LOGW(TAG, "Sensor is not yet initialized");
    }
}