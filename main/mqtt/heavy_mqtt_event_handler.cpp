#include "heavy_mqtt_event_handler.hpp"

#include <esp_log.h>

namespace {
struct Message {
    std::string topic;
    std::string payload;
};
}  // namespace

HeavyMqttEventHandler::HeavyMqttEventHandler(const char* taskName,
                                             uint32_t stackSize,
                                             UBaseType_t priority) {
    queue = xQueueCreate(4, sizeof(Message));
    if (!queue) {
        ESP_LOGE("HeavyMqttEventHandler", "Failed to create queue");
        return;
    }
    xTaskCreate(&HeavyMqttEventHandler::taskFunc, taskName, stackSize, this,
                priority, nullptr);
}

HeavyMqttEventHandler::~HeavyMqttEventHandler() {
    if (queue) {
        vQueueDelete(queue);
    }
}

void HeavyMqttEventHandler::operate(const std::string& topic,
                                    const std::string& payload) {
    Message msg{topic, payload};
    if (xQueueSend(queue, &msg, 0) != pdTRUE) {
        ESP_LOGW("HeavyMqttEventHandler", "Queue full, dropping message for %s",
                 topic.c_str());
    }
}

void HeavyMqttEventHandler::taskFunc(void* arg) {
    auto* self = static_cast<HeavyMqttEventHandler*>(arg);
    Message msg;
    for (;;) {
        if (xQueueReceive(self->queue, &msg, portMAX_DELAY)) {
            self->handleHeavy(msg.topic, msg.payload);
        }
    }
}
