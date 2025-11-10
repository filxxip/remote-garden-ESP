#include "adc_sensor_client.hpp"

#include "esp_log.h"

namespace {
const char* TAG = "AdcSensorClient";
}

AdcSensorClient::AdcSensorClient(std::shared_ptr<MqttClient> mqttClient,
                                 const SensorType type,
                                 const adc_unit_t adcUnit,
                                 const adc_channel_t adcChannel)
    : SensorClient(std::move(mqttClient), type),
      channel(adcChannel),
      adcUnit(adcUnit) {}

AdcSensorClient::~AdcSensorClient() {
    if (adcHandle) {
        adc_oneshot_del_unit(adcHandle);
    }
}

bool AdcSensorClient::initializeImpl() {
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = adcUnit,
        .clk_src = static_cast<adc_oneshot_clk_src_t>(0),
        .ulp_mode = ADC_ULP_MODE_DISABLE};

    esp_err_t err = adc_oneshot_new_unit(&init_config, &adcHandle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create ADC unit: %s", esp_err_to_name(err));
        return false;
    }

    adc_oneshot_chan_cfg_t chan_config = {.atten = ADC_ATTEN_DB_12,
                                          .bitwidth = ADC_BITWIDTH_DEFAULT};

    err = adc_oneshot_config_channel(adcHandle, channel, &chan_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure ADC channel: %s",
                 esp_err_to_name(err));
        adc_oneshot_del_unit(adcHandle);
        adcHandle = nullptr;
        return false;
    }

    ESP_LOGI(TAG, "ADC initialized successfully (unit=%d, channel=%d)", adcUnit,
             channel);
    return true;
}

float AdcSensorClient::readValue() {
    int raw = 0;
    auto err = adc_oneshot_read(adcHandle, channel, &raw);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "ADC read failed: %s", esp_err_to_name(err));
        return -1.0f;
    }
    return convertRawToValue(raw);
}
