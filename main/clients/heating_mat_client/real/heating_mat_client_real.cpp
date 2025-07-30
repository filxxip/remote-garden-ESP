#include "heating_mat_client_real.hpp"

#include <driver/gpio.h>
#include <esp_log.h>

namespace {
constexpr const char* TAG = "HeatingMatClientReal";
static constexpr gpio_num_t HEATING_MAT_GPIO = GPIO_NUM_18;  //@TODO
}  // namespace

HeatingMatClientReal::HeatingMatClientReal(
    std::shared_ptr<MqttClient> mqttClient, int gardenId)
    : ActuatorClient(std::move(mqttClient), "heating_mat", gardenId) {
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << HEATING_MAT_GPIO);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
}

bool HeatingMatClientReal::performTurnOn() {
    ESP_LOGI(TAG, "Turning on real heating mat device");
    return gpio_set_level(HEATING_MAT_GPIO, 1) == ESP_OK;
}

bool HeatingMatClientReal::performTurnOff() {
    ESP_LOGI(TAG, "Turning off real heating mat device");
    return gpio_set_level(HEATING_MAT_GPIO, 0) == ESP_OK;
}
