#include "http_manager.hpp"

#include <esp_http_client.h>
#include <esp_log.h>
#include <stdlib.h>

#include "utils/file_manager/file_manager.hpp"

HttpManager::HttpManager(const std::string& baseUrl) : baseUrl(baseUrl) {}

HttpManager& HttpManager::path(const std::string& path) {
    path_ = path;
    return *this;
}

HttpManager& HttpManager::header(const std::string& key,
                                 const std::string& value) {
    headers[key] = value;
    return *this;
}

HttpManager& HttpManager::body(const std::string& body) {
    body_ = body;
    return *this;
}

HttpManager& HttpManager::contentType(const std::string& type) {
    headers.emplace("Content-Type", type);
    return *this;
}

bool HttpManager::get(std::string& response, const std::string& logTag) {
    return performRequest(HTTP_METHOD_GET, response, logTag);
}

bool HttpManager::post(std::string& response, const std::string& logTag) {
    return performRequest(HTTP_METHOD_POST, response, logTag);
}

bool HttpManager::postAndSaveToFile(const std::string& filePath,
                                    const std::string& logTag) {
    std::string response;
    if (!post(response, logTag)) {
        return false;
    }
    if (!FileManager::writeToFile(filePath, response)) {
        ESP_LOGE(logTag.c_str(), "Failed to save response to file: %s",
                 filePath.c_str());
        return false;
    }
    ESP_LOGI(logTag.c_str(), "Saved response to %s (%d bytes)",
             filePath.c_str(), (int)response.size());
    return true;
}

bool HttpManager::performRequest(esp_http_client_method_t method,
                                 std::string& responseOut,
                                 const std::string& logTag) {
    std::string url = baseUrl + path_;

    esp_http_client_config_t config = {};
    config.url = url.c_str();
    config.timeout_ms = 5000;

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        ESP_LOGE(logTag.c_str(), "Failed to initialize HTTP client");
        return false;
    }

    for (const auto& [key, value] : headers) {
        esp_http_client_set_header(client, key.c_str(), value.c_str());
    }

    esp_http_client_set_method(client, method);
    if (method == HTTP_METHOD_POST && !body_.empty()) {
        esp_http_client_set_post_field(client, body_.c_str(), body_.length());
    }

    esp_err_t err = esp_http_client_perform(client);
    if (err != ESP_OK) {
        ESP_LOGE(logTag.c_str(), "Request failed: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return false;
    }

    int status = esp_http_client_get_status_code(client);
    ESP_LOGI(logTag.c_str(), "HTTP status %d", status);

    std::string response;
    char buffer[256];
    while (true) {
        int read_len = esp_http_client_read(client, buffer, sizeof(buffer) - 1);
        if (read_len <= 0) break;
        buffer[read_len] = '\0';
        response.append(buffer, read_len);
    }

    if (response.empty()) {
        ESP_LOGW(logTag.c_str(), "No response body received");
    } else {
        ESP_LOGI(logTag.c_str(), "Received %d bytes", (int)response.size());
    }

    responseOut = response;

    esp_http_client_cleanup(client);
    return (status >= 200 && status < 300);
}
