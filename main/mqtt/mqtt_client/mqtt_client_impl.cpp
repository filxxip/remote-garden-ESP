#include "mqtt_client_impl.hpp"

#include <esp_log.h>

static const char* TAG = "MQTT";

MqttClient::MqttClient() : client(nullptr) {}

MqttClient::~MqttClient() {
    if (client) {
        esp_mqtt_client_destroy(client);
    }
}

void MqttClient::init(const char* broker_uri) {
    esp_mqtt_client_config_t mqtt_cfg = {};
    mqtt_cfg.broker.address.uri = broker_uri;

    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, MQTT_EVENT_ANY, mqtt_event_handler,
                                   this);
    esp_mqtt_client_start(client);
}

void MqttClient::subscribe(const std::string& topic,
                           std::shared_ptr<MqttEventHandler> handler) {
    std::lock_guard<std::mutex> lock(handlersMutex);

    handlers[topic] = handler;
    esp_mqtt_client_subscribe(client, topic.c_str(), 1);

    ESP_LOGI(TAG, "Subscribed to topic %s", topic.c_str());
}

void MqttClient::publish(const std::string& topic, const std::string& message) {
    if (client) {
        int msg_id = esp_mqtt_client_publish(client, topic.c_str(),
                                             message.c_str(), 0, 1, 0);
        ESP_LOGI(TAG, "Published message id %d to topic %s", msg_id,
                 topic.c_str());
    }
}

void MqttClient::mqtt_event_handler(void* handler_args, esp_event_base_t base,
                                    int32_t event_id, void* event_data) {
    auto* self = static_cast<MqttClient*>(handler_args);
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;

    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_DATA: {
            std::string topic(event->topic, event->topic_len);
            std::string payload(event->data, event->data_len);
            self->handleMessage(topic, payload);
            break;
        }
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT connected");
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT disconnected");
            break;
        default:
            break;
    }
}

void MqttClient::handleMessage(const std::string& topic,
                               const std::string& payload) {
    std::lock_guard<std::mutex> lock(handlersMutex);
    auto it = handlers.find(topic);
    if (it != handlers.end() && it->second) {
        it->second->operate(topic, payload);
    } else {
        ESP_LOGW(TAG, "No handler registered for topic %s", topic.c_str());
    }
}
