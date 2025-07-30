#include "fan_client_real.hpp"

#include <driver/gpio.h>
#include <esp_log.h>

namespace {
constexpr const char* TAG = "FanClientReal";
static constexpr gpio_num_t FAN_GPIO = GPIO_NUM_17;  //@TODO
}  // namespace

FanClientReal::FanClientReal(std::shared_ptr<MqttClient> mqttClient,
                             int gardenId)
    : ActuatorClient(std::move(mqttClient), "fan", gardenId) {
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << FAN_GPIO);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
}

bool FanClientReal::performTurnOn() {
    ESP_LOGI(TAG, "Turning on real fan device");
    return gpio_set_level(FAN_GPIO, 1) == ESP_OK;
}

bool FanClientReal::performTurnOff() {
    ESP_LOGI(TAG, "Turning off real fan device");
    return gpio_set_level(FAN_GPIO, 0) == ESP_OK;
}
