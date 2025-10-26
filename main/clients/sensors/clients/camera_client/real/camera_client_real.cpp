#include "camera_client_real.hpp"

#include <esp_log.h>

#include "driver/i2c.h"
#include "esp_camera.h"

namespace {
constexpr const char* TAG = "CameraSensorClient";

// AXP313A Power Management IC address
constexpr uint8_t AXP313A_ADDR = 0x36;

// Pin configuration for DFRobot FireBeetle 2 ESP32-S3 (DFR0975-U)
constexpr int PIN_PWDN = -1;
constexpr int PIN_RESET = -1;

// I2C pins - SHARED between camera and AXP313A
constexpr int PIN_SIOD = 1;  // IO1 = SDA (shared)
constexpr int PIN_SIOC = 2;  // IO2 = SCL (shared)

// Clock pins
constexpr int PIN_XCLK = 45;  // IO45 = XMCLK
constexpr int PIN_PCLK = 5;   // IO5 = PCLK

// Sync pins
constexpr int PIN_VSYNC = 6;  // IO6 = VSYNC
constexpr int PIN_HREF = 42;  // IO42 = HREF

// Data pins
constexpr int PIN_D2 = 39;
constexpr int PIN_D3 = 40;
constexpr int PIN_D4 = 41;
constexpr int PIN_D5 = 4;
constexpr int PIN_D6 = 7;
constexpr int PIN_D7 = 8;
constexpr int PIN_D8 = 46;
constexpr int PIN_D9 = 48;

constexpr int XCLK_FREQ_HZ = 10000000;  // 20MHz - reduces FB-OVF errors
constexpr framesize_t FRAME_SIZE = FRAMESIZE_96X96;  // Back to QQVGA
constexpr int JPEG_QUALITY = 15;                     // Better quality
constexpr int FB_COUNT = 2;                          // 2 buffers

const camera_config_t CAMERA_CONFIGGGGG = {
    .pin_pwdn = PIN_PWDN,
    .pin_reset = PIN_RESET,
    .pin_xclk = PIN_XCLK,
    .pin_sccb_sda = PIN_SIOD,
    .pin_sccb_scl = PIN_SIOC,

    // Mapowanie D2–D9 → pin_d0–pin_d7
    .pin_d7 = PIN_D9,
    .pin_d6 = PIN_D8,
    .pin_d5 = PIN_D7,
    .pin_d4 = PIN_D6,
    .pin_d3 = PIN_D5,
    .pin_d2 = PIN_D4,
    .pin_d1 = PIN_D3,
    .pin_d0 = PIN_D2,

    .pin_vsync = PIN_VSYNC,
    .pin_href = PIN_HREF,
    .pin_pclk = PIN_PCLK,

    .xclk_freq_hz = XCLK_FREQ_HZ,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_RGB565,
    .frame_size = FRAME_SIZE,
    .jpeg_quality = JPEG_QUALITY,
    .fb_count = FB_COUNT,
    .fb_location = CAMERA_FB_IN_PSRAM,
    .grab_mode = CAMERA_GRAB_LATEST,  // Only grab when buffer empty
    .sccb_i2c_port = 0                // Camera uses I2C_NUM_0
};

static bool powerInitialized = false;
static bool i2cManuallyInstalled = false;

// Manual I2C initialization for AXP313A BEFORE camera init
bool initI2CForPower() {
    if (i2cManuallyInstalled) {
        ESP_LOGI(TAG, "I2C already manually installed");
        return true;
    }

    ESP_LOGI(TAG, "Ręczna inicjalizacja I2C dla AXP313A...");

    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = PIN_SIOD;
    conf.scl_io_num = PIN_SIOC;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 100000;

    esp_err_t err = i2c_param_config(I2C_NUM_0, &conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "i2c_param_config failed: %s", esp_err_to_name(err));
        return false;
    }

    err = i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0);
    if (err == ESP_OK) {
        i2cManuallyInstalled = true;
        ESP_LOGI(TAG, "✓ I2C zainstalowane ręcznie na porcie 0");
        return true;
    } else if (err == ESP_ERR_INVALID_STATE) {
        ESP_LOGI(TAG, "I2C już zainstalowane");
        i2cManuallyInstalled = true;
        return true;
    } else {
        ESP_LOGE(TAG, "i2c_driver_install failed: %s", esp_err_to_name(err));
        return false;
    }
}

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
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);

    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "  AXP313A Reg 0x%02X = 0x%02X", reg, data);
    } else {
        ESP_LOGE(TAG, "  AXP313A Reg 0x%02X read failed", reg);
    }
}

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

void CameraSensorClient::scanI2CBus() {
    ESP_LOGI(TAG, "=== Skanowanie magistrali I2C ===");
    ESP_LOGI(TAG, "Szukam urządzeń I2C na pinach SDA=%d, SCL=%d...", PIN_SIOD,
             PIN_SIOC);

    int devices_found = 0;
    for (uint8_t addr = 1; addr < 127; addr++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);

        esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(50));
        i2c_cmd_link_delete(cmd);

        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "  -> Znaleziono urządzenie na adresie: 0x%02X",
                     addr);
            devices_found++;
            if (addr == 0x30 || addr == 0x21)
                ESP_LOGI(TAG, "     ✓✓✓ To jest kamera OV2640! ✓✓✓");
            else if (addr == 0x36)
                ESP_LOGI(TAG, "     (AXP313A Power Management IC)");
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    if (devices_found == 0) {
        ESP_LOGE(TAG, "✗ Nie znaleziono żadnych urządzeń I2C!");
        ESP_LOGE(TAG, "Sprawdź połączenie kabla kamery");
    } else {
        ESP_LOGI(TAG, "✓ Znaleziono %d urządzeń I2C", devices_found);
    }

    ESP_LOGI(TAG, "=== Koniec skanowania I2C ===");
}

bool CameraSensorClient::enableCameraPower() {
    if (powerInitialized) {
        ESP_LOGI(TAG, "Zasilanie kamery już włączone");
        return true;
    }

    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "╔══════════════════════════════════════╗");
    ESP_LOGI(TAG, "║  WŁĄCZANIE ZASILANIA KAMERY (AXP313A)║");
    ESP_LOGI(TAG, "╚══════════════════════════════════════╝");

    // Initialize I2C manually if not done yet
    if (!initI2CForPower()) {
        ESP_LOGE(TAG, "Failed to initialize I2C for power management");
        return false;
    }

    // Scan I2C bus to verify AXP313A is present
    ESP_LOGI(TAG, "Sprawdzanie obecności AXP313A...");
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (AXP313A_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(100));
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
            i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(100));
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
    i2c_master_cmd_begin(I2C_NUM_0, read_cmd, pdMS_TO_TICKS(100));
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
        i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(100));
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

    powerInitialized = true;
    ESP_LOGI(TAG, "✓ Zasilanie kamery włączone (ALDO1=2.8V, DLDO1=2.8V)");
    return true;
}

void CameraSensorClient::readCameraSensorID() {
    ESP_LOGI(TAG, "=== Odczyt ID sensora kamery ===");

    // Try both possible OV2640 addresses
    uint8_t sensor_addresses[] = {0x30, 0x21};

    for (uint8_t sensor_addr : sensor_addresses) {
        ESP_LOGI(TAG, "Próba odczytu z adresu 0x%02X...", sensor_addr);

        uint8_t midh = 0, midl = 0;

        // Read MIDH (0x0A)
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (sensor_addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write_byte(cmd, 0x0A, true);
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (sensor_addr << 1) | I2C_MASTER_READ, true);
        i2c_master_read_byte(cmd, &midh, I2C_MASTER_NACK);
        i2c_master_stop(cmd);
        esp_err_t ret =
            i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(100));
        i2c_cmd_link_delete(cmd);

        if (ret == ESP_OK)
            ESP_LOGI(TAG, "  ✓ MIDH = 0x%02X", midh);
        else
            ESP_LOGW(TAG, "  ✗ MIDH read failed (0x%x)", ret);

        // Read MIDL (0x0B)
        cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (sensor_addr << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write_byte(cmd, 0x0B, true);
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (sensor_addr << 1) | I2C_MASTER_READ, true);
        i2c_master_read_byte(cmd, &midl, I2C_MASTER_NACK);
        i2c_master_stop(cmd);
        ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(100));
        i2c_cmd_link_delete(cmd);

        if (ret == ESP_OK)
            ESP_LOGI(TAG, "  ✓ MIDL = 0x%02X", midl);
        else
            ESP_LOGW(TAG, "  ✗ MIDL read failed (0x%x)", ret);

        uint16_t sensor_id = (midh << 8) | midl;

        if (sensor_id == 0x2642) {
            ESP_LOGI(TAG, "✓✓✓ KAMERA OV2640 WYKRYTA NA 0x%02X! (ID=0x%04X)",
                     sensor_addr, sensor_id);
            ESP_LOGI(TAG, "=== Koniec odczytu ID ===");
            return;
        }
    }

    ESP_LOGW(TAG, "⚠ KAMERA NIE ODPOWIADA NA ŻADNYM ADRESIE!");
    ESP_LOGE(TAG, "");
    ESP_LOGE(TAG, "╔══════════════════════════════════════╗");
    ESP_LOGE(TAG, "║  BŁĄD: KAMERA NIE ZOSTAŁA WYKRYTA   ║");
    ESP_LOGE(TAG, "╚══════════════════════════════════════╝");
    ESP_LOGE(TAG, "Możliwe przyczyny:");
    ESP_LOGE(TAG, "1. Kabel kamery nie jest podłączony");
    ESP_LOGE(TAG, "2. Kabel kamery jest uszkodzony");
    ESP_LOGE(TAG, "3. Kamera nie otrzymuje zasilania z AXP313A");
    ESP_LOGE(TAG, "4. Kamera jest uszkodzona");
    ESP_LOGE(TAG, "");
    ESP_LOGI(TAG, "=== Koniec odczytu ID ===");
}
bool CameraSensorClient::initializeCamera() {
    if (isCameraInitialized()) {
        ESP_LOGW(TAG, "Camera already initialized");
        return true;
    }

    ESP_LOGI(TAG, "╔══════════════════════════════════════╗");
    ESP_LOGI(TAG, "║  INICJALIZACJA KAMERY OV2640        ║");
    ESP_LOGI(TAG, "╚══════════════════════════════════════╝");

    // CRITICAL: Enable power FIRST, before camera init!
    if (!enableCameraPower()) {
        ESP_LOGE(TAG, "✗ Failed to enable camera power");
        return false;
    }

    // Scan I2C to verify AXP313A is present
    scanI2CBus();

    // Wait for camera power to stabilize
    ESP_LOGI(TAG, "Czekam 500ms na stabilizację kamery...");
    vTaskDelay(pdMS_TO_TICKS(3000));

    // Remove manually installed I2C driver
    if (i2cManuallyInstalled) {
        ESP_LOGI(TAG,
                 "Usuwam ręczny sterownik I2C, aby kamera mogła go przejąć...");
        i2c_driver_delete(I2C_NUM_0);
        i2cManuallyInstalled = false;
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    // Initialize camera
    ESP_LOGI(TAG, "Inicjalizacja esp_camera...");
    ESP_LOGI(TAG, "DEBUG: sccb_i2c_port = %d", CAMERA_CONFIGGGGG.sccb_i2c_port);

    esp_err_t err = esp_camera_init(&CAMERA_CONFIGGGGG);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "✗ esp_camera_init() failed: %s", esp_err_to_name(err));
        return false;
    }

    // === 🔧 DODANE POPRAWKI DLA KOLORU I FLIP ===
    sensor_t* s = esp_camera_sensor_get();
    if (!s) {
        ESP_LOGE(TAG, "✗ esp_camera_sensor_get() returned NULL");
        esp_camera_deinit();
        return false;
    }

    // Daj ISP czas na ustabilizowanie
    vTaskDelay(pdMS_TO_TICKS(300));

    // Włącz flipy i mirror, z pełną korekcją HREF/VREF (poprawka zielonego
    // odcienia)
    s->set_vflip(s, 1);
    s->set_hmirror(s, 1);

    // Włącz automatyczny balans bieli i korekcje optyczne
    s->set_whitebal(s, 1);
    s->set_wb_mode(s, 3);        // neutralne kolory
    s->set_gain_ctrl(s, 0);      // wyłącz automatyczny gain
    s->set_exposure_ctrl(s, 0);  // wyłącz auto-exposure
    s->set_lenc(s, 1);           // włącz lens correction

    // Wyrzuć 3 pierwsze ramki (często prześwietlone / złe kolory)
    for (int i = 0; i < 3; i++) {
        camera_fb_t* fb = esp_camera_fb_get();
        if (fb) esp_camera_fb_return(fb);
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    // === KONIEC POPRAWEK ===

    s->set_framesize(s, FRAME_SIZE);
    s->set_quality(s, JPEG_QUALITY);

    setCameraInitialized(true);
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "╔══════════════════════════════════════╗");
    ESP_LOGI(TAG, "║     ✓✓✓ KAMERA DZIAŁA! ✓✓✓         ║");
    ESP_LOGI(TAG, "╚══════════════════════════════════════╝");
    ESP_LOGI(TAG, "");

    // === TEST RAMEK ===
    ESP_LOGI(TAG, "=== TEST RAMEK - próba pobrania 10 ramek ===");
    int good_frames = 0;
    int bad_frames = 0;

    for (int i = 0; i < 10; i++) {
        ESP_LOGI(TAG, "Test %d/10: Czekam na ramkę...", i + 1);
        camera_fb_t* fb = esp_camera_fb_get();

        if (fb) {
            if (fb->len > 100) {
                ESP_LOGI(
                    TAG,
                    "  ✓ Ramka OK: %d bajtów, format: 0x%02X%02X...%02X%02X",
                    fb->len, fb->buf[0], fb->buf[1], fb->buf[fb->len - 2],
                    fb->buf[fb->len - 1]);
                good_frames++;
            } else {
                ESP_LOGW(TAG, "  ✗ Ramka zbyt mała: %d bajtów", fb->len);
                bad_frames++;
            }
            esp_camera_fb_return(fb);
        } else {
            ESP_LOGE(TAG, "  ✗ Timeout - brak ramki!");
            bad_frames++;
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }

    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "Poprawne ramki: %d/10", good_frames);
    ESP_LOGI(TAG, "Uszkodzone:     %d/10", bad_frames);
    ESP_LOGI(TAG, "");

    // Czyszczenie początkowych ramek
    ESP_LOGI(TAG, "Czekam 2 sekundy i czyszczę początkowe ramki...");
    vTaskDelay(pdMS_TO_TICKS(2000));

    camera_fb_t* discard_fb = nullptr;
    int discarded = 0;
    while ((discard_fb = esp_camera_fb_get()) != nullptr && discarded < 10) {
        esp_camera_fb_return(discard_fb);
        discarded++;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    ESP_LOGI(TAG, "✓ Wyczyszczono %d początkowych ramek", discarded);

    return true;
}

void CameraSensorClient::cleanupCamera() {
    if (!isCameraInitialized()) return;
    esp_camera_deinit();
    setCameraInitialized(false);
    powerInitialized = false;
    ESP_LOGI(TAG, "Camera deinitialized");
}

bool CameraSensorClient::captureImage(std::vector<uint8_t>& outImage) {
    ESP_LOGI(TAG, "Przechwytywanie ŚWIEŻEGO zdjęcia...");

    // KRYTYCZNE: Wyrzuć starą ramkę (pre-wypełnioną)
    camera_fb_t* old_fb = esp_camera_fb_get();
    if (old_fb) {
        esp_camera_fb_return(old_fb);
        ESP_LOGI(TAG, "✓ Wyrzucono starą ramkę");
    }

    // TERAZ pobierz ŚWIEŻĄ ramkę
    vTaskDelay(pdMS_TO_TICKS(100));  // Daj czas na nową ramkę

    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAG, "✗ Nie udało się pobrać świeżej ramki");
        return false;
    }

    // Sprawdź poprawność
    if (fb->len < 100) {
        ESP_LOGW(TAG, "✗ Ramka zbyt mała (%d bajtów)", fb->len);
        esp_camera_fb_return(fb);
        return false;
    }

    // Success!
    outImage.assign(fb->buf, fb->buf + fb->len);
    ESP_LOGI(TAG, "✓ Świeże zdjęcie: %d bajtów", fb->len);
    esp_camera_fb_return(fb);
    return true;
}