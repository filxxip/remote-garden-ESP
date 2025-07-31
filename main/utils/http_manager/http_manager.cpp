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

    esp_http_client_config_t config = {
        .url = url.c_str(),
        .timeout_ms = 5000,
    };

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

    int content_length = esp_http_client_get_content_length(client);
    char* buffer = (char*)malloc(content_length + 1);
    if (!buffer) {
        ESP_LOGE(logTag.c_str(), "Failed to allocate buffer");
        esp_http_client_cleanup(client);
        return false;
    }

    int read_len = esp_http_client_read(client, buffer, content_length);
    buffer[read_len] = '\0';
    responseOut.assign(buffer, read_len);
    free(buffer);

    esp_http_client_cleanup(client);
    return true;
}
