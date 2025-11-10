#include "camera_client_real.hpp"

#include "robot.hpp"

namespace {
constexpr const char* TAG = "CameraSensorClient";
}

#define CONFIG_CAMERA_JPEG_MODE

CameraSensorClient::CameraSensorClient(std::shared_ptr<MqttClient> mqttClient,
                                       SensorType type, size_t maxCacheSize)
    : BaseCameraSensorClient(std::move(mqttClient), type, maxCacheSize) {}

bool CameraSensorClient::initializeImpl() {
    if (isInitialized()) {
        ESP_LOGI(TAG, "Camera already initialized");
        return true;
    }

    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = GPIO_NUM_1;
    conf.scl_io_num = GPIO_NUM_2;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 100000;
    i2c_param_config(I2C_NUM_0, &conf);
    i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0);
    begin(I2C_NUM_0, 0x36);
    vTaskDelay(pdMS_TO_TICKS(3000));

    disablePower();
    vTaskDelay(pdMS_TO_TICKS(3000));
    enableCameraPower(OV2640);
    vTaskDelay(pdMS_TO_TICKS(3000));

    i2c_driver_delete(I2C_NUM_0);
    vTaskDelay(pdMS_TO_TICKS(3000));

    ESP_LOGI(TAG, "Initializing FireBeetle 2 ESP32-S3 camera...");

    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = 39;
    config.pin_d1 = 40;
    config.pin_d2 = 41;
    config.pin_d3 = 4;
    config.pin_d4 = 7;
    config.pin_d5 = 8;
    config.pin_d6 = 46;
    config.pin_d7 = 48;
    config.pin_xclk = 45;
    config.pin_pclk = 5;
    config.pin_vsync = 6;
    config.pin_href = 42;
    config.pin_sccb_sda = 1;
    config.pin_sccb_scl = 2;
    config.pin_pwdn = -1;
    config.pin_reset = -1;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;

    // config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 10;
    config.fb_count = 1;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.grab_mode = CAMERA_GRAB_LATEST;

    config.frame_size = FRAMESIZE_VGA;
    // config.fb_location = CAMERA_FB_IN_DRAM;
    ESP_LOGW(TAG, "No PSRAM found, lowering frame size");
    // }

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed: 0x%x", err);
        return false;
    }
    vTaskDelay(pdMS_TO_TICKS(5000));

    ESP_LOGI(TAG, "Camera initialized successfully");
    return true;
}

void CameraSensorClient::cleanupCamera() {
    if (!isInitialized()) return;

    esp_camera_deinit();
    ESP_LOGI(TAG, "Camera deinitialized");
}

bool CameraSensorClient::captureImage(std::vector<uint8_t>& outImage) {
    if (!isInitialized()) {
        ESP_LOGE(TAG, "Camera not initialized");
        return false;
    }

    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAG, "Failed to capture frame buffer");
        return false;
    }

    // // uint8_t* jpg_buf = nullptr;
    // // size_t jpg_len = 0;

    // // bool converted = fmt2jpg(fb->buf, fb->len, fb->width, fb->height,
    // //                          fb->format, 80, &jpg_buf, &jpg_len);

    // esp_camera_fb_return(fb);

    // // if (!converted || !jpg_buf) {
    // //     ESP_LOGE(TAG, "JPEG conversion failed");
    // //     if (jpg_buf) free(jpg_buf);
    // //     return false;
    // // }

    // // if (jpg_len >= 2 && jpg_buf[0] == 0xFF && jpg_buf[1] == 0xD8) {
    // //     outImage.assign(jpg_buf, jpg_buf + jpg_len);
    // //     free(jpg_buf);
    // //     ESP_LOGI(TAG, "Converted to JPEG: %zu bytes (from %zu RGB565)",
    // //              outImage.size(), fb->len);
    // //     return true;
    // // }

    // // ESP_LOGE(TAG, "Invalid JPEG after conversion");
    // // free(jpg_buf);
    // // return false;
    // return true;

    // camera_fb_t* fb = esp_camera_fb_get();
    // if (!fb) {
    //     ESP_LOGE(TAG, "Failed to capture frame buffer");
    //     return false;
    // }

    ESP_LOGI(TAG, "Captured frame: %d bytes, format=%d", fb->len, fb->format);

    if (fb->len < 2 || fb->buf[0] != 0xFF || fb->buf[1] != 0xD8) {
        ESP_LOGE(TAG, "Invalid JPEG start marker: %02X %02X", fb->buf[0],
                 fb->buf[1]);
        esp_camera_fb_return(fb);
        return false;
    }

    // gotowy JPEG — kopiujemy do outImage
    outImage.assign(fb->buf, fb->buf + fb->len);

    // teraz dopiero zwracamy bufor do sterownika
    esp_camera_fb_return(fb);

    ESP_LOGI(TAG, "✓ Captured JPEG frame (%zu bytes)", outImage.size());
    return true;
}
