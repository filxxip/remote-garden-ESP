#include "dht_11_client.hpp"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rom/ets_sys.h"

namespace {
const char* TAG = "Dht11Reader";
}

Dht11Reader::Dht11Reader(gpio_num_t pin)
    : dataPin(pin), temperature(-1.0f), humidity(-1.0f) {
    gpio_set_direction(dataPin, GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_set_pull_mode(dataPin, GPIO_PULLUP_ONLY);
    gpio_set_level(dataPin, 1);
    vTaskDelay(pdMS_TO_TICKS(1000));
    ESP_LOGI(TAG, "Initialized DHT11 on GPIO %d", dataPin);
}

bool Dht11Reader::waitForLevel(int level, int timeout_us) {
    for (int i = 0; i < timeout_us; i++) {
        if (gpio_get_level(dataPin) == level) {
            return true;
        }
        ets_delay_us(1);
    }
    return false;
}

uint8_t Dht11Reader::readByte() {
    uint8_t byte = 0;

    for (int i = 0; i < 8; i++) {
        if (!waitForLevel(1, 100)) {
            return 0;
        }

        ets_delay_us(40);

        if (gpio_get_level(dataPin) == 1) {
            byte |= (1 << (7 - i));
        }

        if (!waitForLevel(0, 100)) {
            return 0;
        }
    }

    return byte;
}

esp_err_t Dht11Reader::readSensor(uint8_t data[5]) {
    gpio_set_direction(dataPin, GPIO_MODE_OUTPUT);
    gpio_set_level(dataPin, 0);
    vTaskDelay(pdMS_TO_TICKS(20));

    gpio_set_level(dataPin, 1);
    ets_delay_us(30);
    gpio_set_direction(dataPin, GPIO_MODE_INPUT);
    ets_delay_us(10);

    if (!waitForLevel(0, 100)) {
        return ESP_ERR_TIMEOUT;
    }

    if (!waitForLevel(1, 100)) {
        return ESP_ERR_TIMEOUT;
    }

    if (!waitForLevel(0, 100)) {
        return ESP_ERR_TIMEOUT;
    }

    for (int i = 0; i < 5; i++) {
        data[i] = readByte();
    }

    uint8_t checksum = data[0] + data[1] + data[2] + data[3];
    if (checksum != data[4]) {
        return ESP_ERR_INVALID_CRC;
    }

    return ESP_OK;
}

bool Dht11Reader::update() {
    uint8_t data[5] = {0};

    ESP_LOGI(TAG, "=== Reading DHT11 on GPIO %d ===", dataPin);

    esp_err_t res = readSensor(data);

    if (res == ESP_OK) {
        humidity = (float)data[0];
        temperature = (float)data[2];

        ESP_LOGI(
            TAG, "DHT11 SUCCESS: Temp=%.1f°C Hum=%.1f%% [%d %d %d %d cs=%d]",
            temperature, humidity, data[0], data[1], data[2], data[3], data[4]);
        return true;
    }

    if (res == ESP_ERR_TIMEOUT) {
        ESP_LOGW(TAG, "TIMEOUT - DHT11 not responding on GPIO %d", dataPin);
    } else if (res == ESP_ERR_INVALID_CRC) {
        ESP_LOGW(TAG, "CHECKSUM ERROR [%d %d %d %d cs=%d expected=%d]", data[0],
                 data[1], data[2], data[3], data[4],
                 (uint8_t)(data[0] + data[1] + data[2] + data[3]));
    }

    return false;
}

float Dht11Reader::getTemperature() const { return temperature; }
float Dht11Reader::getHumidity() const { return humidity; }