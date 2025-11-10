#pragma once

#include <atomic>
#include <memory>
#include <vector>

#include "clients/sensors/clients/sensor_client.hpp"
#include "polling_manager/polling_manager.hpp"

class SensorPollingManager : public PollingManager {
   public:
    SensorPollingManager(std::vector<std::shared_ptr<SensorClient>> sensors,
                         int intervalMs = 5000, int memory = 3072);

    void pollAction() const;
    virtual void action(const std::shared_ptr<SensorClient>& sensor) const = 0;

   private:
    std::vector<std::shared_ptr<SensorClient>> sensors;
};
