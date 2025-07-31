#include "device_provisioner.hpp"

#include <esp_log.h>

#include "utils/crypto/crypto.hpp"
#include "utils/file_manager/file_manager.hpp"
#include "utils/http_manager/http_manager.hpp"

namespace {
constexpr const char* TAG = "PROVISION";
constexpr const char* DEVICE_ID_PATH = "/spiffs/device_id.txt";
constexpr const char* DEVICE_SECRET_PATH = "/spiffs/device_secret.txt";
constexpr const char* CLIENT_KEY_PATH = "/spiffs/client.key";
constexpr const char* CLIENT_CERT_PATH = "/spiffs/client.crt";
constexpr const char* CA_CERT_PATH = "/spiffs/ca.crt";

bool readFile(const char* path, std::string& out) {
    return FileManager::readFromFile(path, out);
}

bool saveKeyAndCert(const std::string& key, const std::string& cert) {
    return FileManager::writeToFile(CLIENT_KEY_PATH, key) &&
           FileManager::writeToFile(CLIENT_CERT_PATH, cert);
}

bool sendCsrAndGetCert(const std::string& deviceId,
                       const std::string& deviceSecret,
                       const std::string& csrPem, std::string& certOut) {
    return HttpManager("https://your.server.url")
        .path("/register")
        .contentType("application/x-pem-file")
        .header("X-Device-ID", deviceId)
        .header("X-Device-Secret", deviceSecret)
        .body(csrPem)
        .post(certOut, TAG);
}

}  // anonymous namespace

bool DeviceProvisioner::isProvisioned() {
    return FileManager::fileExists(CLIENT_KEY_PATH) &&
           FileManager::fileExists(CLIENT_CERT_PATH);
}

bool DeviceProvisioner::ensureProvisioning() {
    if (isProvisioned()) {
        ESP_LOGI(TAG, "Already provisioned");
        return true;
    }

    std::string deviceId, deviceSecret;
    if (!readFile(DEVICE_ID_PATH, deviceId) ||
        !readFile(DEVICE_SECRET_PATH, deviceSecret)) {
        ESP_LOGE(TAG, "Missing device_id or secret");
        return false;
    }

    std::string privateKey, csrPem;
    if (!Crypto::generateRsaKeyAndCsr(deviceId, privateKey, csrPem)) {
        ESP_LOGE(TAG, "Failed to generate CSR");
        return false;
    }

    std::string signedCert;
    if (!sendCsrAndGetCert(deviceId, deviceSecret, csrPem, signedCert)) {
        ESP_LOGE(TAG, "Failed to register and get cert");
        return false;
    }

    if (!saveKeyAndCert(privateKey, signedCert)) {
        ESP_LOGE(TAG, "Failed to store cert and key");
        return false;
    }

    ESP_LOGI(TAG, "Provisioning complete");
    return true;
}

DeviceProvisioner::TlsCred DeviceProvisioner::loadTlsCredentials() {
    std::string key, cert, ca, device;
    if (!FileManager::readFromFile(CLIENT_KEY_PATH, key)) {
        ESP_LOGE(TAG, "Failed to load client.key");
    }
    if (!FileManager::readFromFile(CLIENT_CERT_PATH, cert)) {
        ESP_LOGE(TAG, "Failed to load client.crt");
    }
    if (!FileManager::readFromFile(CA_CERT_PATH, ca)) {
        ESP_LOGE(TAG, "Failed to load ca.crt");
    }
    if (!FileManager::readFromFile(DEVICE_ID_PATH, device)) {
        ESP_LOGE(TAG, "Failed to load device_id.txt");
    }
    return {key, cert, ca, device};
}
