#include "sensor_reader.hpp"

void SensorReader::action(const std::shared_ptr<SensorClient>& sensor) const {
    sensor->storeReading();
}