#include "file_manager.hpp"

#include <esp_log.h>
#include <esp_random.h>
#include <sys/stat.h>
#include <sys/unistd.h>

#include <cstdio>
#include <cstring>

namespace {
constexpr const char* TAG = "FILE_MANAGER";
}

namespace FileManager {

bool writeToFile(const std::string& path, const std::string& content) {
    FILE* file = fopen(path.c_str(), "w");
    if (!file) {
        ESP_LOGE(TAG, "Failed to open file for writing: %s", path.c_str());
        return false;
    }

    size_t bytesWritten = fwrite(content.c_str(), 1, content.size(), file);
    fclose(file);

    if (bytesWritten != content.size()) {
        ESP_LOGE(TAG, "Failed to write all content to file: %s", path.c_str());
        return false;
    }

    ESP_LOGI(TAG, "Wrote %d bytes to %s", (int)content.size(), path.c_str());
    return true;
}

bool readFromFile(const std::string& path, std::string& content) {
    FILE* file = fopen(path.c_str(), "r");
    if (!file) {
        ESP_LOGE(TAG, "Failed to open file for reading: %s", path.c_str());
        return false;
    }

    // Find file size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (fileSize <= 0) {
        fclose(file);
        ESP_LOGW(TAG, "File is empty or error getting size: %s", path.c_str());
        content.clear();
        return true;
    }

    content.resize(fileSize);
    size_t bytesRead = fread(&content[0], 1, fileSize, file);
    fclose(file);

    content.resize(bytesRead);

    ESP_LOGI(TAG, "Read %d bytes from %s", (int)bytesRead, path.c_str());
    return bytesRead > 0;
}

bool fileExists(const std::string& path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

bool deleteFile(const std::string& path) {
    if (remove(path.c_str()) == 0) {
        ESP_LOGI(TAG, "Deleted file: %s", path.c_str());
        return true;
    } else {
        ESP_LOGE(TAG, "Failed to delete file: %s", path.c_str());
        return false;
    }
}

bool secureDeleteFile(const std::string& path) {
    ESP_LOGI(TAG, "Secure delete file: %s", path.c_str());

    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        ESP_LOGW(TAG, "File not found for secure delete: %s", path.c_str());
        return false;
    }

    long size = st.st_size;
    if (size <= 0) {
        ESP_LOGW(TAG, "File is empty or invalid size: %s", path.c_str());
        return deleteFile(path);
    }

    // Wzorce nadpisywania zgodne z DoD 5220.22-M standard
    uint8_t patterns[] = {
        0x00,  // Wszystkie zera
        0xFF,  // Wszystkie jedynki
        0xAA,  // 10101010
        0x55,  // 01010101
        0x33,  // 00110011
        0xCC   // 11001100
    };

    // KROK 1: Wielokrotne nadpisywanie wzorcami
    for (uint8_t pattern : patterns) {
        std::string patternData(size, pattern);

        if (!writeToFile(path, patternData)) {
            ESP_LOGE(TAG, "Failed to overwrite with pattern 0x%02X: %s",
                     pattern, path.c_str());
            return false;
        }

        // Wymuś zapis na flash (ważne!)
        FILE* f = fopen(path.c_str(), "r");
        if (f) {
            fsync(fileno(f));
            fclose(f);
        }

        ESP_LOGD(TAG, "Overwrite pass 0x%02X completed", pattern);
    }

    // KROK 2: Finalne nadpisanie losowymi danymi (3 razy)
    for (int pass = 0; pass < 3; pass++) {
        std::string randomData;
        randomData.resize(size);

        // Generuj prawdziwie losowe dane
        for (long i = 0; i < size; i++) {
            randomData[i] = static_cast<char>(esp_random() & 0xFF);
        }

        if (!writeToFile(path, randomData)) {
            ESP_LOGE(TAG, "Failed random overwrite pass %d: %s", pass + 1,
                     path.c_str());
            return false;
        }

        // Wymuś zapis
        FILE* f = fopen(path.c_str(), "r");
        if (f) {
            fsync(fileno(f));
            fclose(f);
        }

        ESP_LOGD(TAG, "Random overwrite pass %d completed", pass + 1);
    }

    // KROK 3: Usuń plik
    bool result = deleteFile(path);

    if (result) {
        ESP_LOGI(TAG, "Secure delete completed: %s", path.c_str());
    }

    return result;
}

}  // namespace FileManager