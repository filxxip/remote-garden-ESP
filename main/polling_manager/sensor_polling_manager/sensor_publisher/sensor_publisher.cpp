#include "sensor_publisher.hpp"

void SensorPublisher::action(
    const std::shared_ptr<SensorClient>& sensor) const {
    sensor->flushCache();
}