#pragma once

#include <atomic>
#include <memory>
#include <vector>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

class PollingManager {
   public:
    PollingManager(int intervalMs = 5000);
    virtual ~PollingManager();

    virtual void pollAction() const = 0;
    void start();
    void stop();
    void resume();

   private:
    static void pollingTask(void* pvParameters);

    int intervalMs;
    TaskHandle_t taskHandle = nullptr;
    std::atomic<bool> isStopped{false};
};
