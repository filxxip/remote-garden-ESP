#include "light_sensor_client_real.hpp"

#include "esp_log.h"

static const char* TAG = "LightSensorClient";

LightSensorClientReal::LightSensorClientReal(
    std::shared_ptr<MqttClient> mqttClient, int gardenId, adc_unit_t adcUnit,
    adc_channel_t adcChannel)
    : SensorClient(mqttClient, "light", gardenId), channel(adcChannel) {
    adc_oneshot_unit_init_cfg_t init_config;
    init_config.unit_id = adcUnit;
    init_config.clk_src = static_cast<adc_oneshot_clk_src_t>(0);  // default
    init_config.ulp_mode = ADC_ULP_MODE_DISABLE;

    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adcHandle));

    adc_oneshot_chan_cfg_t chan_config;
    chan_config.bitwidth = ADC_BITWIDTH_DEFAULT;
    chan_config.atten = ADC_ATTEN_DB_12;

    ESP_ERROR_CHECK(
        adc_oneshot_config_channel(adcHandle, channel, &chan_config));
}

LightSensorClientReal::~LightSensorClientReal() {
    if (adcHandle) {
        adc_oneshot_del_unit(adcHandle);
    }
}

float LightSensorClientReal::readValue() {
    int raw = 0;
    esp_err_t err = adc_oneshot_read(adcHandle, channel, &raw);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "ADC read failed: %s", esp_err_to_name(err));
        return -1.0f;
    }

    float voltage = (static_cast<float>(raw) / 4095.0f) * 3.3f;
    return voltage;
}
