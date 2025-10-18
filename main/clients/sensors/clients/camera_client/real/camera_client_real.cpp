#include "camera_client_real.hpp"

#include <esp_log.h>

#include "esp_camera.h"

namespace {
constexpr const char* TAG = "CameraSensorClient";

// Pin configuration for DFRobot FireBeetle 2 ESP32-S3 (DFR0975-U)
constexpr int PIN_PWDN = -1;
constexpr int PIN_RESET = -1;
constexpr int PIN_XCLK = 45;
constexpr int PIN_SIOD = 1;
constexpr int PIN_SIOC = 2;
constexpr int PIN_D7 = 48;
constexpr int PIN_D6 = 46;
constexpr int PIN_D5 = 8;
constexpr int PIN_D4 = 7;
constexpr int PIN_D3 = 4;
constexpr int PIN_D2 = 41;
constexpr int PIN_D1 = 40;
constexpr int PIN_D0 = 39;
constexpr int PIN_VSYNC = 6;
constexpr int PIN_HREF = 42;
constexpr int PIN_PCLK = 5;

constexpr int XCLK_FREQ_HZ = 20000000;
constexpr framesize_t FRAME_SIZE = FRAMESIZE_VGA;
constexpr int JPEG_QUALITY = 15;
constexpr int FB_COUNT = 2;

const camera_config_t CAMERA_CONFIG = {.pin_pwdn = PIN_PWDN,
                                       .pin_reset = PIN_RESET,
                                       .pin_xclk = PIN_XCLK,
                                       .pin_sccb_sda = PIN_SIOD,
                                       .pin_sccb_scl = PIN_SIOC,

                                       .pin_d7 = PIN_D7,
                                       .pin_d6 = PIN_D6,
                                       .pin_d5 = PIN_D5,
                                       .pin_d4 = PIN_D4,
                                       .pin_d3 = PIN_D3,
                                       .pin_d2 = PIN_D2,
                                       .pin_d1 = PIN_D1,
                                       .pin_d0 = PIN_D0,
                                       .pin_vsync = PIN_VSYNC,
                                       .pin_href = PIN_HREF,
                                       .pin_pclk = PIN_PCLK,

                                       .xclk_freq_hz = XCLK_FREQ_HZ,
                                       .ledc_timer = LEDC_TIMER_0,
                                       .ledc_channel = LEDC_CHANNEL_0,

                                       .pixel_format = PIXFORMAT_JPEG,
                                       .frame_size = FRAME_SIZE,
                                       .jpeg_quality = JPEG_QUALITY,
                                       .fb_count = FB_COUNT,
                                       .fb_location = CAMERA_FB_IN_PSRAM,
                                       .grab_mode = CAMERA_GRAB_LATEST};
}  // namespace

CameraSensorClient::CameraSensorClient(std::shared_ptr<MqttClient> mqttClient,
                                       const SensorType type)
    : BaseCameraSensorClient(std::move(mqttClient), type) {
    ESP_LOGI(TAG, "Camera sensor client created");
}

CameraSensorClient::~CameraSensorClient() {
    if (isCameraInitialized()) {
        cleanupCamera();
    }
}

bool CameraSensorClient::initializeCamera() {
    if (isCameraInitialized()) {
        ESP_LOGW(TAG, "Camera already initialized");
        return true;
    }

    esp_err_t err = esp_camera_init(&CAMERA_CONFIG);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed with error 0x%x", err);
        return false;
    }

    sensor_t* s = esp_camera_sensor_get();
    if (s == NULL) {
        ESP_LOGE(TAG, "Failed to get camera sensor");
        esp_camera_deinit();
        return false;
    }

    // OV2640 sensor configuration
    s->set_brightness(s, 0);
    s->set_contrast(s, 0);
    s->set_saturation(s, 0);
    s->set_special_effect(s, 0);
    s->set_whitebal(s, 1);
    s->set_awb_gain(s, 1);
    s->set_wb_mode(s, 0);
    s->set_exposure_ctrl(s, 1);
    s->set_aec2(s, 0);
    s->set_ae_level(s, 0);
    s->set_aec_value(s, 300);
    s->set_gain_ctrl(s, 1);
    s->set_agc_gain(s, 0);
    s->set_gainceiling(s, (gainceiling_t)0);
    s->set_bpc(s, 0);
    s->set_wpc(s, 1);
    s->set_raw_gma(s, 1);
    s->set_lenc(s, 1);
    s->set_hmirror(s, 0);
    s->set_vflip(s, 0);
    s->set_dcw(s, 1);
    s->set_colorbar(s, 0);

    setCameraInitialized(true);
    ESP_LOGI(TAG, "Camera initialized successfully");
    return true;
}

void CameraSensorClient::cleanupCamera() {
    if (!isCameraInitialized()) {
        return;
    }

    esp_camera_deinit();
    setCameraInitialized(false);
    ESP_LOGI(TAG, "Camera deinitialized");
}

bool CameraSensorClient::captureImage(std::vector<uint8_t>& outImage) {
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAG, "Camera capture failed");
        return false;
    }

    outImage.assign(fb->buf, fb->buf + fb->len);

    esp_camera_fb_return(fb);
    return true;
}