#include "atomize_client_real.hpp"

#include <driver/gpio.h>
#include <esp_log.h>

namespace {
constexpr const char* TAG = "AtomizeClientReal";
constexpr gpio_num_t ATOMIZE_GPIO = GPIO_NUM_17;  // @TODO configure properly
}  // namespace

AtomizeClientReal::AtomizeClientReal(std::shared_ptr<MqttClient> mqttClient,
                                     int gardenId)
    : ActuatorClient(std::move(mqttClient), "atomize", gardenId) {
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << ATOMIZE_GPIO);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
}

bool AtomizeClientReal::performTurnOn() {
    ESP_LOGI(TAG, "Turning on real atomizer device");
    return gpio_set_level(ATOMIZE_GPIO, 1) == ESP_OK;
}

bool AtomizeClientReal::performTurnOff() {
    ESP_LOGI(TAG, "Turning off real atomizer device");
    return gpio_set_level(ATOMIZE_GPIO, 0) == ESP_OK;
}