#include "sensor_publisher_reader.hpp"

void SensorPublisherReader::action(
    const std::shared_ptr<SensorClient>& sensor) const {
    sensor->storeReading();
    sensor->flushCache();
}