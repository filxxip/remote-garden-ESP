#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include <string>

#include "mqtt_event_handler.hpp"

class HeavyMqttEventHandler : public MqttEventHandler {
   public:
    HeavyMqttEventHandler(const char* taskName = "heavy_handler",
                          uint32_t stackSize = 4096, UBaseType_t priority = 5);
    ~HeavyMqttEventHandler() override;

    void operate(const std::string& topic,
                 const std::string& payload) override final;

   protected:
    virtual void handleHeavy(const std::string& topic,
                             const std::string& payload) = 0;

   private:
    static void taskFunc(void* arg);
    QueueHandle_t queue;
};
