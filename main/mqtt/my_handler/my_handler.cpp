#include "my_handler.hpp"

#include <esp_log.h>

void MyHandler::operate(const std::string& topic, const std::string& payload) {
    ESP_LOGI("MyHandler", "Received message on %s: %s", topic.c_str(),
             payload.c_str());
}