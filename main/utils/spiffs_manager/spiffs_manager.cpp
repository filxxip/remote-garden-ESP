#include "spiffs_manager.hpp"

#include <esp_log.h>
#include <esp_random.h>
#include <esp_spiffs.h>
#include <mbedtls/aes.h>
#include <nvs.h>
#include <nvs_flash.h>

#include "utils/file_manager/file_manager.hpp"

namespace {
constexpr const char* TAG = "SPIFFS_ENC";
constexpr const char* NVS_NAMESPACE = "spiffs";
constexpr const char* NVS_KEY_NAME = "aes_key";

static uint8_t g_aes_key[32];

bool load_aes_key_from_nvs() {
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS: %s", esp_err_to_name(err));
        return false;
    }

    size_t size = sizeof(g_aes_key);
    err = nvs_get_blob(handle, NVS_KEY_NAME, g_aes_key, &size);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "AES key not found, generating new one...");
        for (size_t i = 0; i < sizeof(g_aes_key); i += 4) {
            uint32_t rnd = esp_random();
            memcpy(&g_aes_key[i], &rnd, 4);
        }
        err = nvs_set_blob(handle, NVS_KEY_NAME, g_aes_key, sizeof(g_aes_key));
        if (err == ESP_OK) {
            nvs_commit(handle);
            ESP_LOGI(TAG, "AES key generated and stored in NVS");
        }
    } else if (err == ESP_OK) {
        ESP_LOGI(TAG, "AES key loaded from NVS");
    }

    nvs_close(handle);
    return true;
}

std::string encrypt(const std::string& plain) {
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_enc(&aes, g_aes_key, 256);

    uint8_t iv[16] = {0};
    size_t nc_off = 0;
    unsigned char stream_block[16];

    std::string out(plain.size(), '\0');
    mbedtls_aes_crypt_ctr(&aes, plain.size(), &nc_off, iv, stream_block,
                          (const uint8_t*)plain.data(), (uint8_t*)out.data());
    mbedtls_aes_free(&aes);
    return out;
}

std::string decrypt(const std::string& cipher) {
    // AES-CTR decrypt == encrypt
    return encrypt(cipher);
}

std::string toSpiffsPath(const std::string& path) {
    if (path.rfind("/spiffs/", 0) == 0) {
        return path;
    }
    return "/spiffs/" + path;
}
}  // namespace

namespace SpiffsManager {

bool init() {
    ESP_LOGI(TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {.base_path = "/spiffs",
                                  .partition_label = "spiffs",
                                  .max_files = 5,
                                  .format_if_mount_failed = true};

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPIFFS mount failed: %s", esp_err_to_name(ret));
        return false;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info("spiffs", &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS info: %s", esp_err_to_name(ret));
        return false;
    }

    ESP_LOGI(TAG, "SPIFFS mounted. total=%d, used=%d", total, used);

    return load_aes_key_from_nvs();
}

void deinit() {
    esp_vfs_spiffs_unregister("spiffs");
    ESP_LOGI(TAG, "SPIFFS deinitialized");
}

bool writeEncrypted(const std::string& path, const std::string& plain) {
    std::string cipher = encrypt(plain);
    return FileManager::writeToFile(toSpiffsPath(path), cipher);
}

bool readEncrypted(const std::string& path, std::string& plainOut) {
    std::string cipher;
    if (!FileManager::readFromFile(toSpiffsPath(path), cipher)) {
        return false;
    }
    plainOut = decrypt(cipher);
    return true;
}

bool readBlank(const std::string& path, std::string& plainOut) {
    return FileManager::readFromFile(toSpiffsPath(path), plainOut);
}

bool deleteFile(const std::string& path) {
    return FileManager::secureDeleteFile(toSpiffsPath(path));
}

bool fileExists(const std::string& path) {
    return FileManager::fileExists(toSpiffsPath(path));
}

}  // namespace SpiffsManager
