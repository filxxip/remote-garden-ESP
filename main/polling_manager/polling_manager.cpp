#include "polling_manager.hpp"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "esp_log.h"

namespace {
constexpr const char* TAG = "PollingManager";
}

PollingManager::PollingManager(int intervalMs) : intervalMs(intervalMs) {}

PollingManager::~PollingManager() {
    if (taskHandle) {
        vTaskDelete(taskHandle);
    }
}

void PollingManager::start() {
    xTaskCreatePinnedToCore(pollingTask, "PollingTask", 5120, this, 5,
                            &taskHandle, 1);
}

void PollingManager::pollingTask(void* pvParameters) {
    auto* self = static_cast<PollingManager*>(pvParameters);

    while (true) {
        if (!self->isStopped) {
            self->pollAction();
        }
        vTaskDelay(pdMS_TO_TICKS(self->intervalMs));
    }
}

void PollingManager::stop() {
    ESP_LOGI(TAG, "Stopping polling task");
    isStopped = true;
}

void PollingManager::resume() {
    ESP_LOGI(TAG, "Resuming polling task");
    isStopped = false;
}
