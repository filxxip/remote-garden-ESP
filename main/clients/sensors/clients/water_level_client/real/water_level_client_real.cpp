#include "water_level_client_real.hpp"

#include "esp_log.h"

namespace {
const char* TAG = "WaterLevelClientReal";
constexpr uint8_t ATTINY_LOW_ADDR = 0x77;
constexpr uint8_t ATTINY_HIGH_ADDR = 0x78;
constexpr int THRESHOLD = 80;
}  // namespace

WaterLevelClientReal::WaterLevelClientReal(
    std::shared_ptr<MqttClient> mqttClient, i2c_port_t i2cPort)
    : SensorClient(std::move(mqttClient), SensorType::WaterLevel),
      port(i2cPort) {
    ESP_LOGI(TAG, "ATTiny water level sensor on I2C port %d", port);
}

float WaterLevelClientReal::readValue() {
    std::array<uint8_t, 8> low{};
    std::array<uint8_t, 12> high{};

    if (i2c_master_read_from_device(port, ATTINY_LOW_ADDR, low.data(),
                                    low.size(),
                                    100 / portTICK_PERIOD_MS) != ESP_OK ||
        i2c_master_read_from_device(port, ATTINY_HIGH_ADDR, high.data(),
                                    high.size(),
                                    100 / portTICK_PERIOD_MS) != ESP_OK) {
        ESP_LOGW(TAG, "Failed to read ATTiny sensors");
        return -1.0f;
    }

    int activeLow = 0, activeHigh = 0;
    for (auto v : low)
        if (v > THRESHOLD) activeLow++;
    for (auto v : high)
        if (v > THRESHOLD) activeHigh++;

    int total = low.size() + high.size();
    float percent =
        ((activeLow + activeHigh) / static_cast<float>(total)) * 100.0f;

    ESP_LOGI(TAG, "Water level: %.1f%%", percent);
    return percent;
}