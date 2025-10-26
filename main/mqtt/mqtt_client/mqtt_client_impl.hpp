#pragma once

#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "mqtt/mqtt_event_handler.hpp"
#include "mqtt_client.h"

class MqttClient {
   public:
    MqttClient();
    ~MqttClient();

    void init(const char* brokerUri);
    void subscribe(const std::string& topic,
                   std::shared_ptr<MqttEventHandler> handler);
    void unsubscribe(const std::string& topic);
    bool publish(const std::string& topic, const std::string& message);
    bool publishRaw(const std::string& topic, const std::vector<uint8_t>& data);
    void registerOnConnected(std::function<void(MqttClient&)> callback);
    void registerOnDisconnected(std::function<void(MqttClient&)> callback);

    void start();
    void stop();

   private:
    esp_mqtt_client_handle_t client;
    std::unordered_map<std::string, std::shared_ptr<MqttEventHandler>> handlers;
    std::mutex handlersMutex;
    std::function<void(MqttClient&)> onConnectedCallback;
    std::function<void(MqttClient&)> onDisconnectedCallback;
    std::optional<std::string> topicPrefix;

    std::string clientKey;
    std::string clientCert;
    std::string caCert;
    std::string deviceId;

    static void mqtt_event_handler(void* handler_args, esp_event_base_t base,
                                   int32_t event_id, void* event_data);

    void handleMessage(const std::string& topic, const std::string& payload);
};
