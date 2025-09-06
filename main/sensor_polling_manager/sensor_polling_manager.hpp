#pragma once

#include <atomic>
#include <memory>
#include <vector>

#include "clients/sensors/clients/sensor_client.hpp"

class SensorPollingManager {
   public:
    SensorPollingManager(std::vector<std::shared_ptr<SensorClient>> sensors,
                         int intervalMs = 5000);
    ~SensorPollingManager();

    virtual void action(const std::shared_ptr<SensorClient>& sensor) const = 0;
    void stop();
    void resume();

   private:
    static void pollingTask(void* pvParameters);

    std::vector<std::shared_ptr<SensorClient>> sensors;
    int intervalMs;
    TaskHandle_t taskHandle = nullptr;
    std::atomic<bool> isStopped{false};
};
