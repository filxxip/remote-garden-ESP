#pragma once

#include <esp_http_client.h>

#include <map>
#include <string>

class HttpManager {
   public:
    explicit HttpManager(const std::string& baseUrl);

    HttpManager& path(const std::string& path);
    HttpManager& header(const std::string& key, const std::string& value);
    HttpManager& body(const std::string& body);
    HttpManager& contentType(const std::string& type);

    bool get(std::string& response, const std::string& logTag = "HTTP");
    bool post(std::string& response, const std::string& logTag = "HTTP");

   private:
    bool performRequest(esp_http_client_method_t method,
                        std::string& responseOut, const std::string& logTag);

    std::string baseUrl;
    std::string path_;
    std::string body_;
    std::map<std::string, std::string> headers;
};
