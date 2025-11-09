#include "camera_client_real.hpp"

#include "driver/i2c.h"
#include "esp_camera.h"
#include "robot.hpp"
// #include "esp_psram.h"
namespace {
constexpr const char* TAG = "CameraSensorClient";
constexpr uint8_t AXP313A_ADDR = 0x36;
void readAXPRegister(uint8_t reg) {
    uint8_t data = 0;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (AXP313A_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (AXP313A_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, &data, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_1, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);

    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "  AXP313A Reg 0x%02X = 0x%02X", reg, data);
    } else {
        ESP_LOGE(TAG, "  AXP313A Reg 0x%02X read failed", reg);
    }
}
}  // namespace

// Konstruktor
CameraSensorClient::CameraSensorClient(std::shared_ptr<MqttClient> mqttClient,
                                       SensorType type, size_t maxCacheSize)
    : BaseCameraSensorClient(std::move(mqttClient), type, maxCacheSize) {}

// Inicjalizacja kamery
bool CameraSensorClient::initializeCamera() {
    ESP_LOGI(TAG, "initializeCamera() called");
    if (isCameraInitialized()) {
        ESP_LOGI(TAG, "Camera already initialized");
        return true;
    }

    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = 1;
    conf.scl_io_num = 2;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 100000;
    esp_err_t err = i2c_param_config(I2C_NUM_1, &conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "i2c_param_config failed: %s", esp_err_to_name(err));
        // return false;
    }

    err = i2c_driver_install(I2C_NUM_1, conf.mode, 0, 0, 0);
    if (err == ESP_OK) {
        // i2cManuallyInstalled = true;
        ESP_LOGI(TAG, "✓ I2C zainstalowane ręcznie na porcie 0");
        // return true;
    } else if (err == ESP_ERR_INVALID_STATE) {
        ESP_LOGI(TAG, "I2C już zainstalowane");
        // i2cManuallyInstalled = true;
        // return true;
    } else {
        ESP_LOGE(TAG, "i2c_driver_install failed: %s", esp_err_to_name(err));
        // return false;
    }

    // Scan I2C bus to verify AXP313A is present
    ESP_LOGI(TAG, "Sprawdzanie obecności AXP313A...");
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (AXP313A_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_1, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "✗ AXP313A nie odpowiada na adresie 0x%02X (błąd: %s)",
                 AXP313A_ADDR, esp_err_to_name(ret));
        return false;
    }
    ESP_LOGI(TAG, "✓ AXP313A znaleziony na adresie 0x%02X", AXP313A_ADDR);

    // Read AXP313A registers before configuration
    ESP_LOGI(TAG, "=== Stan rejestrów AXP313A PRZED konfiguracją ===");
    readAXPRegister(0x00);  // Power status
    readAXPRegister(0x10);  // Output control
    readAXPRegister(0x90);  // ALDO1 voltage
    readAXPRegister(0x17);  // DLDO1 voltage

    // Configure AXP313A - MANUAL REGISTER WRITES with correct addresses
    ESP_LOGI(TAG, "Ręczna konfiguracja rejestrów AXP313A...");

    auto writeReg = [](uint8_t reg, uint8_t value) -> esp_err_t {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (AXP313A_ADDR << 1) | I2C_MASTER_WRITE,
                              true);
        i2c_master_write_byte(cmd, reg, true);
        i2c_master_write_byte(cmd, value, true);
        i2c_master_stop(cmd);
        esp_err_t ret =
            i2c_master_cmd_begin(I2C_NUM_1, cmd, pdMS_TO_TICKS(100));
        i2c_cmd_link_delete(cmd);
        return ret;
    };

    // Step 1: Read current state to preserve other rails
    ESP_LOGI(TAG, "Odczyt aktualnego stanu wyjść...");
    uint8_t current_state = 0;
    i2c_cmd_handle_t read_cmd = i2c_cmd_link_create();
    i2c_master_start(read_cmd);
    i2c_master_write_byte(read_cmd, (AXP313A_ADDR << 1) | I2C_MASTER_WRITE,
                          true);
    i2c_master_write_byte(read_cmd, 0x10, true);
    i2c_master_start(read_cmd);
    i2c_master_write_byte(read_cmd, (AXP313A_ADDR << 1) | I2C_MASTER_READ,
                          true);
    i2c_master_read_byte(read_cmd, &current_state, I2C_MASTER_NACK);
    i2c_master_stop(read_cmd);
    i2c_master_cmd_begin(I2C_NUM_1, read_cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(read_cmd);
    ESP_LOGI(TAG, "Aktualny stan 0x10 = 0x%02X", current_state);

    // Step 2: Set voltages for camera rails
    ESP_LOGI(TAG, "Konfiguracja napięć dla kamery...");

    // DCDC3: 1.8V for camera digital core (DVDD)
    // Formula: (1800 - 500) / 100 = 13 = 0x0D
    // CRITICAL: Write multiple times to ensure it sticks!
    for (int i = 0; i < 3; i++) {
        if (writeReg(0x80, 0x0D) != ESP_OK) {
            ESP_LOGE(TAG, "Failed to set DCDC3 voltage (attempt %d)", i + 1);
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    ESP_LOGI(TAG, "✓ DCDC3 = 1.8V (zapisano 3x)");
    vTaskDelay(pdMS_TO_TICKS(50));

    // ALDO1: 2.8V for camera analog (AVDD)
    if (writeReg(0x90, 0x17) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set ALDO1 voltage");
        return false;
    }
    ESP_LOGI(TAG, "✓ ALDO1 = 2.8V");
    vTaskDelay(pdMS_TO_TICKS(50));

    // DLDO1: 2.8V for camera I/O (DOVDD)
    // Real DLDO1 register is 0x17, NOT 0x99!
    if (writeReg(0x17, 0x17) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set DLDO1 voltage");
        return false;
    }
    ESP_LOGI(TAG, "✓ DLDO1 = 2.8V");
    vTaskDelay(pdMS_TO_TICKS(50));

    // Step 3: Enable all camera power rails
    ESP_LOGI(TAG, "Włączam wyjścia zasilania...");

    // Enable DCDC3 (bit 2), ALDO1 (bit 3), DLDO1 (bit 4) = 0x1C
    // But preserve other bits that might already be enabled
    uint8_t new_state = current_state | 0x1C;
    if (writeReg(0x10, new_state) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable power rails");
        return false;
    }
    ESP_LOGI(TAG, "✓ Wszystkie zasilacze włączone (0x%02X)", new_state);
    vTaskDelay(pdMS_TO_TICKS(300));

    // Step 4: Wait for power to stabilize
    ESP_LOGI(TAG, "Czekam 1000ms na stabilizację zasilania...");
    vTaskDelay(pdMS_TO_TICKS(1000));

    // Read AXP313A registers after configuration
    ESP_LOGI(TAG, "=== Stan rejestrów AXP313A PO konfiguracji ===");
    readAXPRegister(0x00);  // Power status
    readAXPRegister(0x10);  // Output control
    readAXPRegister(0x90);  // ALDO1 voltage
    readAXPRegister(0x17);  // DLDO1 voltage

    // Verify voltages are set correctly
    uint8_t aldo1_val = 0, dldo1_val = 0, ctrl_val = 0;

    auto readReg = [](uint8_t reg) -> uint8_t {
        uint8_t val = 0;
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (AXP313A_ADDR << 1) | I2C_MASTER_WRITE,
                              true);
        i2c_master_write_byte(cmd, reg, true);
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (AXP313A_ADDR << 1) | I2C_MASTER_READ, true);
        i2c_master_read_byte(cmd, &val, I2C_MASTER_NACK);
        i2c_master_stop(cmd);
        i2c_master_cmd_begin(I2C_NUM_1, cmd, pdMS_TO_TICKS(100));
        i2c_cmd_link_delete(cmd);
        return val;
    };

    ctrl_val = readReg(0x10);
    aldo1_val = readReg(0x90);
    dldo1_val = readReg(0x17);

    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "╔══════════════════════════════════════╗");
    ESP_LOGI(TAG, "║       WERYFIKACJA ZASILANIA         ║");
    ESP_LOGI(TAG, "╚══════════════════════════════════════╝");
    ESP_LOGI(TAG, "Control (0x10): 0x%02X %s", ctrl_val,
             (ctrl_val & 0x18) == 0x18 ? "[OK - ALDO1+DLDO1]" : "[BŁĄD!]");
    ESP_LOGI(TAG, "ALDO1   (0x90): 0x%02X %s", aldo1_val,
             aldo1_val == 0x17 ? "[OK - 2.8V]" : "[BŁĄD - oczekiwano 0x17]");
    ESP_LOGI(TAG, "DLDO1   (0x17): 0x%02X %s", dldo1_val,
             dldo1_val == 0x17 ? "[OK - 2.8V]" : "[BŁĄD - oczekiwano 0x17]");

    if ((ctrl_val & 0x18) != 0x18 || aldo1_val != 0x17 || dldo1_val != 0x17) {
        ESP_LOGE(TAG, "");
        ESP_LOGE(TAG, "⚠⚠⚠ ZASILANIE KAMERY NIE JEST POPRAWNE! ⚠⚠⚠");
        ESP_LOGE(TAG, "Kamera może nie działać stabilnie.");
        ESP_LOGE(TAG, "");
    } else {
        ESP_LOGI(TAG, "");
        ESP_LOGI(TAG, "✓✓✓ ZASILANIE KAMERY OK! ✓✓✓");
        ESP_LOGI(TAG, "");
    }

    // powerInitialized = true;
    ESP_LOGI(TAG, "✓ Zasilanie kamery włączone (ALDO1=2.8V, DLDO1=2.8V)");
    vTaskDelay(pdMS_TO_TICKS(5000));

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
    // ESP_LOGW(TAG, "No PSRAM found, lowering frame size");
    // }

    err = esp_camera_init(&config);
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
