#pragma once

#include <memory>
#include <vector>

#include "clients/sensor_client.hpp"

class SensorPollingManager {
   public:
    SensorPollingManager(std::vector<std::shared_ptr<SensorClient>> sensors,
                         int intervalMs = 5000);
    ~SensorPollingManager();

   private:
    static void pollingTask(void* pvParameters);

    std::vector<std::shared_ptr<SensorClient>> sensors;
    int intervalMs;
    TaskHandle_t taskHandle = nullptr;
};
