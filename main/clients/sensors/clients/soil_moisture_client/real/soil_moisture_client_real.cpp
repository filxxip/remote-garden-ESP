#include "soil_moisture_client_real.hpp"

#include "esp_log.h"

namespace {
const char* TAG = "SoilMoistureClientReal";
}

SoilMoistureClientReal::SoilMoistureClientReal(
    std::shared_ptr<MqttClient> mqttClient, adc1_channel_t channel)
    : SensorClient(std::move(mqttClient), SensorType::SoilMoisture),
      adcChannel(channel) {}

bool SoilMoistureClientReal::initializeImpl() {
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(adcChannel, ADC_ATTEN_DB_11);
    ESP_LOGI(TAG, "Soil moisture sensor on ADC channel %d", adcChannel);

    return true;
}

float SoilMoistureClientReal::readValue() {
    int raw = adc1_get_raw(adcChannel);
    float percent = 100.0f - ((raw / 4095.0f) * 100.0f);
    ESP_LOGI(TAG, "Soil moisture: %.2f%% (raw=%d)", percent, raw);
    return percent;
}
