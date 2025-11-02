#include "camera_client_real.hpp"

// #include "esp_psram.h"
namespace {
constexpr const char* TAG = "CameraSensorClient";
}

// Konstruktor
CameraSensorClient::CameraSensorClient(std::shared_ptr<MqttClient> mqttClient,
                                       SensorType type, size_t maxCacheSize)
    : BaseCameraSensorClient(std::move(mqttClient), type, maxCacheSize) {}

// Inicjalizacja kamery
bool CameraSensorClient::initializeCamera() {
    if (isCameraInitialized()) {
        ESP_LOGI(TAG, "Camera already initialized");
        return true;
    }

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
    config.jpeg_quality = 15;
    config.fb_count = 1;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.grab_mode = CAMERA_GRAB_LATEST;

    // if (psramFound()) {
    //     config.jpeg_quality = 10;
    //     config.fb_count = 2;
    //     config.fb_location = CAMERA_FB_IN_PSRAM;
    //     ESP_LOGI(TAG, "PSRAM detected, enabling high-quality mode");
    // } else {
    config.frame_size = FRAMESIZE_VGA;
    // config.fb_location = CAMERA_FB_IN_DRAM;
    ESP_LOGW(TAG, "No PSRAM found, lowering frame size");
    // }

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed: 0x%x", err);
        setCameraInitialized(false);
        return false;
    }

    setCameraInitialized(true);
    ESP_LOGI(TAG, "Camera initialized successfully");
    return true;
}

// Sprzątanie
void CameraSensorClient::cleanupCamera() {
    if (!isCameraInitialized()) return;

    esp_camera_deinit();
    setCameraInitialized(false);
    ESP_LOGI(TAG, "Camera deinitialized");
}

// Przechwycenie obrazu
bool CameraSensorClient::captureImage(std::vector<uint8_t>& outImage) {
    if (!isCameraInitialized()) {
        ESP_LOGE(TAG, "Camera not initialized");
        return false;
    }

    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAG, "Failed to capture frame buffer");
        return false;
    }

    outImage.assign(fb->buf, fb->buf + fb->len);
    esp_camera_fb_return(fb);

    ESP_LOGI(TAG, "Captured image: %zu bytes", outImage.size());
    return true;
}
