#pragma once

#include "sensor_polling_manager/sensor_polling_manager.hpp"

class SensorReader final : public SensorPollingManager {
   public:
    using SensorPollingManager::SensorPollingManager;

    void action(const std::shared_ptr<SensorClient>& sensor) const override;
};