#include "sensor_polling_manager.hpp"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

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
        for (const auto& sensor : self->sensors) {
            if (sensor) {
                self->action(sensor);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(self->intervalMs));
    }
}
