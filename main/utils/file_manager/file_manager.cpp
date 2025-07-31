#include "file_manager.hpp"

#include <esp_log.h>
#include <sys/stat.h>

#include <cstdio>
#include <cstring>

namespace FileManager {

bool writeToFile(const std::string& path, const std::string& content) {
    FILE* file = fopen(path.c_str(), "w");
    if (!file) {
        ESP_LOGE("FILE", "Failed to open file for writing: %s", path.c_str());
        return false;
    }

    size_t bytesWritten = fwrite(content.c_str(), 1, content.size(), file);
    fclose(file);

    if (bytesWritten != content.size()) {
        ESP_LOGE("FILE", "Failed to write all content to file: %s",
                 path.c_str());
        return false;
    }

    ESP_LOGI("FILE", "Wrote %d bytes to %s", (int)content.size(), path.c_str());
    return true;
}

bool readFromFile(const std::string& path, std::string& content) {
    FILE* file = fopen(path.c_str(), "r");
    if (!file) {
        ESP_LOGE("FILE", "Failed to open file for reading: %s", path.c_str());
        return false;
    }

    // Find file size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (fileSize <= 0) {
        fclose(file);
        ESP_LOGW("FILE", "File is empty or error getting size: %s",
                 path.c_str());
        content.clear();
        return true;
    }

    content.resize(fileSize);
    size_t bytesRead = fread(&content[0], 1, fileSize, file);
    fclose(file);

    content.resize(bytesRead);

    ESP_LOGI("FILE", "Read %d bytes from %s", (int)bytesRead, path.c_str());
    return bytesRead > 0;
}

bool fileExists(const std::string& path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

}  // namespace FileManager