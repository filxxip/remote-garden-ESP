#include "light_sensor_client_real.hpp"

#include "esp_log.h"

namespace {
const char* TAG = "LightSensorClientReal";

// BH1750 Commands
constexpr uint8_t BH1750_POWER_ON = 0x01;
constexpr uint8_t BH1750_POWER_DOWN = 0x00;
constexpr uint8_t BH1750_CONTINUOUS_HIGH_RES_MODE = 0x10;
constexpr uint8_t BH1750_ADDR = 0x23;  // When ADDR pin is LOW
}  // namespace

LightSensorClientReal::LightSensorClientReal(
    std::shared_ptr<MqttClient> mqttClient, i2c_port_t i2cPort,
    gpio_num_t sdaPin, gpio_num_t sclPin)
    : SensorClient(std::move(mqttClient), SensorType::Light),
      port(i2cPort),
      address(BH1750_ADDR),
      i2c_initialized(false) {
    // Konfiguracja I2C
    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = sdaPin;
    conf.scl_io_num = sclPin;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 100000;  // 100kHz

    ESP_ERROR_CHECK(i2c_param_config(port, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(port, conf.mode, 0, 0, 0));
    i2c_initialized = true;

    // Inicjalizacja BH1750
    writeCommand(BH1750_POWER_ON);
    vTaskDelay(pdMS_TO_TICKS(10));
    writeCommand(BH1750_CONTINUOUS_HIGH_RES_MODE);
    vTaskDelay(pdMS_TO_TICKS(180));  // Wait for first measurement

    ESP_LOGI(TAG, "BH1750 initialized on I2C port %d (SDA=%d, SCL=%d)", port,
             sdaPin, sclPin);
}

LightSensorClientReal::~LightSensorClientReal() {
    if (i2c_initialized) {
        writeCommand(BH1750_POWER_DOWN);
        i2c_driver_delete(port);
    }
}

esp_err_t LightSensorClientReal::writeCommand(uint8_t cmd) {
    i2c_cmd_handle_t handle = i2c_cmd_link_create();
    i2c_master_start(handle);
    i2c_master_write_byte(handle, (address << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(handle, cmd, true);
    i2c_master_stop(handle);

    esp_err_t ret = i2c_master_cmd_begin(port, handle, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(handle);

    return ret;
}

esp_err_t LightSensorClientReal::readLightLevel(uint16_t* lux) {
    uint8_t data[2] = {0};

    i2c_cmd_handle_t handle = i2c_cmd_link_create();
    i2c_master_start(handle);
    i2c_master_write_byte(handle, (address << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(handle, &data[0], I2C_MASTER_ACK);
    i2c_master_read_byte(handle, &data[1], I2C_MASTER_NACK);
    i2c_master_stop(handle);

    esp_err_t ret = i2c_master_cmd_begin(port, handle, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(handle);

    if (ret == ESP_OK) {
        *lux = ((data[0] << 8) | data[1]) /
               1.2;  // Convert to lux (mode dependent)
    }

    return ret;
}

float LightSensorClientReal::readValue() {
    uint16_t lux = 0;
    esp_err_t res = readLightLevel(&lux);

    if (res == ESP_OK) {
        ESP_LOGI(TAG, "Light intensity: %u lx", lux);
        return static_cast<float>(lux);
    }

    ESP_LOGW(TAG, "Failed to read BH1750: %s", esp_err_to_name(res));
    return -1.0f;
}