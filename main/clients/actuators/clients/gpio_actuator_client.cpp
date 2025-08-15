#include "gpio_actuator_client.hpp"

#include <esp_log.h>

namespace {
constexpr const char* TAG = "GpioActuatorClient";
}

GpioActuatorClient::GpioActuatorClient(std::shared_ptr<MqttClient> mqttClient,
                                       const ActuatorType type,
                                       gpio_num_t gpioNum)
    : ActuatorClient(std::move(mqttClient), type), gpio(gpioNum) {
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << gpio);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
}

bool GpioActuatorClient::performTurnOn() {
    ESP_LOGI(TAG, "Turning ON GPIO actuator on GPIO %d", gpio);
    return gpio_set_level(gpio, 1) == ESP_OK;
}

bool GpioActuatorClient::performTurnOff() {
    ESP_LOGI(TAG, "Turning OFF GPIO actuator on GPIO %d", gpio);
    return gpio_set_level(gpio, 0) == ESP_OK;
}
