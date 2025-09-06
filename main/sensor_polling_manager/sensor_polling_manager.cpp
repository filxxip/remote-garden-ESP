#include "sensor_polling_manager.hpp"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "esp_log.h"

namespace {
constexpr const char* TAG = "SensorPollingManager";
}

SensorPollingManager::SensorPollingManager(
    std::vector<std::shared_ptr<SensorClient>> sensors, int intervalMs)
    : sensors(std::move(sensors)), intervalMs(intervalMs) {
    xTaskCreatePinnedToCore(pollingTask, "SensorPollingTask", 4096, this, 5,
                            &taskHandle, tskNO_AFFINITY);
}

SensorPollingManager::~SensorPollingManager() {
    if (taskHandle) {
        vTaskDelete(taskHandle);
    }
}

void SensorPollingManager::pollingTask(void* pvParameters) {
    auto* self = static_cast<SensorPollingManager*>(pvParameters);

    while (true) {
        if (!self->isStopped) {
            for (const auto& sensor : self->sensors) {
                if (sensor) {
                    self->action(sensor);
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(self->intervalMs));
    }
}

void SensorPollingManager::stop() {
    ESP_LOGI(TAG, "Stopping polling task");
    isStopped = true;
}

void SensorPollingManager::resume() {
    ESP_LOGI(TAG, "Resuming polling task");
    isStopped = false;
}
