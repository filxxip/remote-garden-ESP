#include "device_provisioner.hpp"

#include <esp_log.h>

#include "utils/crypto/crypto.hpp"
#include "utils/file_manager/file_manager.hpp"
#include "utils/http_manager/http_manager.hpp"
#include "utils/littlefs_manager/littlefs_manager.hpp"
#include "utils/options/options.hpp"

namespace {
constexpr const char* TAG = "PROVISION";
constexpr const char* DEVICE_ID_PATH = "/littlefs/device_id.txt";
constexpr const char* DEVICE_SECRET_PATH = "/littlefs/device_secret.txt";
constexpr const char* CLIENT_KEY_PATH = "/littlefs/client.key";
constexpr const char* CLIENT_CERT_PATH = "/littlefs/client.crt";
constexpr const char* CA_CERT_PATH = "/littlefs/ca.crt";
constexpr const char* USER_KEY_PATH = "/littlefs/user.txt";

constexpr const char* PATH = "/v1/esp/register";

bool readFile(const char* path, std::string& out) {
    auto result = FileManager::readFromFile(path, out);
    ESP_LOGI(TAG, "Read '%s' from %s", out.c_str(), path);
    return result;
}

bool readEncrypted(const char* path, std::string& out) {
    auto result = FileManager::readFromFile(path, out);
    ESP_LOGI(TAG, "Read '%s' from %s", out.c_str(), path);
    return result;
}

bool saveKeyAndCert(const std::string& key, const std::string& cert) {
    ESP_LOGI(TAG, "Try to save content %s to file %s", key.c_str(),
             CLIENT_KEY_PATH);
    ESP_LOGI(TAG, "Try to save content %s to file %s", cert.c_str(),
             CLIENT_CERT_PATH);
    return FileManager::writeToFile(CLIENT_KEY_PATH, key) &&
           FileManager::writeToFile(CLIENT_CERT_PATH, cert);
}

bool sendCsrAndGetCert(const std::string& deviceId,
                       const std::string& deviceSecret,
                       const std::string& userKey, const std::string& csrPem,
                       std::string& certOut) {
    return HttpManager(Options::getServerUrl())
        .path(PATH)
        .contentType("application/x-pem-file")
        .header("X-Device-ID", deviceId)
        .header("X-Device-Secret", deviceSecret)
        .header("X-User-Key", userKey)
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

    std::string deviceId, deviceSecret, userKey;
    if (!readFile(DEVICE_ID_PATH, deviceId) ||
        !readEncrypted(DEVICE_SECRET_PATH, deviceSecret) ||
        !readFile(USER_KEY_PATH, userKey)) {
        ESP_LOGE(TAG, "Missing device_id, secret, or user_key");
        return false;
    }

    std::string privateKey, csrPem;
    if (!Crypto::generateRsaKeyAndCsr(deviceId, privateKey, csrPem)) {
        ESP_LOGE(TAG, "Failed to generate CSR");
        return false;
    }

    std::string signedCert;
    if (!sendCsrAndGetCert(deviceId, deviceSecret, userKey, csrPem,
                           signedCert)) {
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
    if (!readEncrypted(CLIENT_KEY_PATH, key)) {
        ESP_LOGE(TAG, "Failed to load client.key");
    }
    if (!readEncrypted(CLIENT_CERT_PATH, cert)) {
        ESP_LOGE(TAG, "Failed to load client.crt");
    }
    if (!readEncrypted(CA_CERT_PATH, ca)) {
        ESP_LOGE(TAG, "Failed to load ca.crt");
    }
    if (!FileManager::readFromFile(DEVICE_ID_PATH, device)) {
        ESP_LOGE(TAG, "Failed to load device_id.txt");
    }
    return {key, cert, ca, device};
}

bool DeviceProvisioner::removeProvisioned() {
    bool ok = true;
    ok &= FileManager::deleteFile(CLIENT_KEY_PATH);
    ok &= FileManager::deleteFile(CLIENT_CERT_PATH);
    ok &= FileManager::deleteFile(USER_KEY_PATH);
    return ok;
}