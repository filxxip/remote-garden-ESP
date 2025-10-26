#pragma once

#include "driver/gpio.h"
#include "esp_err.h"

class Dht11Reader {
   public:
    explicit Dht11Reader(gpio_num_t pin);
    bool update();
    float getTemperature() const;
    float getHumidity() const;

   private:
    gpio_num_t dataPin;
    float temperature;
    float humidity;

    esp_err_t readSensor(uint8_t data[5]);
    bool waitForLevel(int level, int timeout_us);
    uint8_t readByte();
};