#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "mqtt/mqtt_event_handler.hpp"
#include "mqtt_client.h"

class MqttClient {
   public:
    MqttClient();
    ~MqttClient();

    void init(const char* broker_uri);
    void subscribe(const std::string& topic,
                   std::shared_ptr<MqttEventHandler> handler);
    void publish(const std::string& topic, const std::string& message);

   private:
    esp_mqtt_client_handle_t client;
    std::unordered_map<std::string, std::shared_ptr<MqttEventHandler>> handlers;
    std::mutex handlersMutex;

    static void mqtt_event_handler(void* handler_args, esp_event_base_t base,
                                   int32_t event_id, void* event_data);

    void handleMessage(const std::string& topic, const std::string& payload);
};
