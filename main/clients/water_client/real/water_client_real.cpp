#include "water_client_real.hpp"

#include <driver/gpio.h>
#include <esp_log.h>

namespace {
constexpr const char* TAG = "WaterClientReal";
static constexpr gpio_num_t WATER_GPIO = GPIO_NUM_5;  //@TODO
}  // namespace

WaterClientReal::WaterClientReal(std::shared_ptr<MqttClient> mqttClient,
                                 int gardenId)
    : ActuatorClient(std::move(mqttClient), "water", gardenId) {
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << WATER_GPIO);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
}

bool WaterClientReal::performTurnOn() {
    ESP_LOGI(TAG, "Turning on real water device");
    return gpio_set_level(WATER_GPIO, 1) == ESP_OK;
}

bool WaterClientReal::performTurnOff() {
    ESP_LOGI(TAG, "Turning off real water device");
    return gpio_set_level(WATER_GPIO, 0) == ESP_OK;
}
