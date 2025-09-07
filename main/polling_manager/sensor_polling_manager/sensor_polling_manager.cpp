#include "sensor_polling_manager.hpp"

SensorPollingManager::SensorPollingManager(
    std::vector<std::shared_ptr<SensorClient>> sensors, int intervalMs)
    : PollingManager(intervalMs), sensors(std::move(sensors)) {}

void SensorPollingManager::pollAction() const {
    for (const auto& sensor : sensors) {
        if (sensor) {
            action(sensor);
        }
    }
}
