#include "http_manager.hpp"

#include <esp_http_client.h>
#include <esp_log.h>
#include <stdlib.h>

#include "utils/file_manager/file_manager.hpp"

namespace {
const char* getMethodName(const esp_http_client_method_t method) {
    switch (method) {
        case HTTP_METHOD_GET:
            return "GET";
        case HTTP_METHOD_POST:
            return "POST";
        case HTTP_METHOD_PUT:
            return "PUT";
        case HTTP_METHOD_PATCH:
            return "PATCH";
        case HTTP_METHOD_DELETE:
            return "DELETE";
        case HTTP_METHOD_HEAD:
            return "HEAD";
        case HTTP_METHOD_OPTIONS:
            return "OPTIONS";
        default:
            return "UNKNOWN";
    }
}
}  // namespace

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
    config.timeout_ms = 10000;
    config.buffer_size = 1024;
    config.buffer_size_tx = 1024;

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        ESP_LOGE(logTag.c_str(), "Failed to initialize HTTP client");
        return false;
    }

    // Ustawiamy nagłówki
    for (const auto& [key, value] : headers) {
        esp_http_client_set_header(client, key.c_str(), value.c_str());
    }

    esp_http_client_set_method(client, method);

    // Sprawdzamy czy metoda może mieć body (POST, PUT, PATCH)
    bool hasBody = (method == HTTP_METHOD_POST || method == HTTP_METHOD_PUT ||
                    method == HTTP_METHOD_PATCH) &&
                   !body_.empty();

    int bodyLength = hasBody ? body_.length() : 0;

    if (hasBody) {
        ESP_LOGI(logTag.c_str(), "Request body size: %d", bodyLength);
    }

    // Otwieramy połączenie
    esp_err_t err = esp_http_client_open(client, bodyLength);
    if (err != ESP_OK) {
        ESP_LOGE(logTag.c_str(), "Failed to open connection: %s",
                 esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return false;
    }

    // Wysyłamy dane jeśli metoda to wymaga
    if (hasBody) {
        int written =
            esp_http_client_write(client, body_.c_str(), body_.length());
        if (written < 0) {
            ESP_LOGE(logTag.c_str(), "Failed to write request body");
            esp_http_client_close(client);
            esp_http_client_cleanup(client);
            return false;
        }
        ESP_LOGI(logTag.c_str(), "Written %d bytes", written);
    }

    // Pobieramy nagłówki odpowiedzi i status
    int content_length = esp_http_client_fetch_headers(client);
    int status = esp_http_client_get_status_code(client);

    ESP_LOGI(logTag.c_str(), "HTTP %s %s -> Status: %d, Content-Length: %d",
             getMethodName(method), url.c_str(), status, content_length);

    // Sprawdzamy status kod
    bool statusOk = (status >= 200 && status < 300);
    if (!statusOk) {
        ESP_LOGW(logTag.c_str(), "HTTP error status: %d", status);
    }

    // Czytamy odpowiedź (nawet jeśli status nie jest OK, może zawierać błąd)
    std::string response;
    if (content_length > 0) {
        response.reserve(content_length);
    }

    char buffer[512];
    int total_read = 0;

    while (true) {
        int read_len = esp_http_client_read(client, buffer, sizeof(buffer));
        if (read_len == 0) {
            // Koniec danych
            break;
        } else if (read_len < 0) {
            ESP_LOGE(logTag.c_str(), "Error reading response: %s",
                     esp_err_to_name(read_len));
            break;
        } else {
            response.append(buffer, read_len);
            total_read += read_len;

            // Jeśli znamy content_length i przeczytaliśmy wszystko
            if (content_length > 0 && total_read >= content_length) {
                break;
            }
        }
    }

    ESP_LOGI(logTag.c_str(), "Received %d bytes", (int)response.size());

    // Debug preview dla niepustych odpowiedzi
    if (!response.empty()) {
        std::string preview =
            response.substr(0, std::min(200, (int)response.size()));
        ESP_LOGI(logTag.c_str(), "Response preview: %s%s", preview.c_str(),
                 response.size() > 200 ? "..." : "");
    } else if (content_length > 0) {
        ESP_LOGW(logTag.c_str(), "Expected %d bytes but got empty response",
                 content_length);
    }

    responseOut = std::move(response);

    esp_http_client_close(client);
    esp_http_client_cleanup(client);

    // Zwracamy true jeśli status OK (niezależnie od tego czy jest response
    // body)
    return statusOk;
}