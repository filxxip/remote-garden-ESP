#include "soil_moisture_client_real.hpp"

#include "esp_log.h"

namespace {
const char* TAG = "SoilMoistureClientReal";
}

SoilMoistureClientReal::SoilMoistureClientReal(
    std::shared_ptr<MqttClient> mqttClient, adc2_channel_t channel)
    : SensorClient(std::move(mqttClient), SensorType::SoilMoisture),
      adcChannel(channel) {}

bool SoilMoistureClientReal::initializeImpl() {
    adc2_config_channel_atten(adcChannel, ADC_ATTEN_DB_11);
    ESP_LOGI(TAG, "Soil moisture sensor on ADC2 channel %d", adcChannel);
    return true;
}

float SoilMoistureClientReal::readValue() {
    int raw = 0;
    esp_err_t err = adc2_get_raw(adcChannel, ADC_WIDTH_BIT_12, &raw);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Failed to read ADC2: %s", esp_err_to_name(err));
        return -1.0f;
    }

    float percent = 100.0f - ((raw / 4095.0f) * 100.0f);
    ESP_LOGI(TAG, "Soil moisture: %.2f%% (raw=%d)", percent, raw);
    return percent;
}
