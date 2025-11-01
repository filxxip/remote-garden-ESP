#include "camera_client.hpp"

#include <esp_log.h>

#include "utils/enum/enum_mapper.hpp"

namespace {
constexpr const char* TAG = "BaseCameraSensorClient";
constexpr const char* IMAGE_TOPIC = "device/camera/frame";
}  // namespace

BaseCameraSensorClient::BaseCameraSensorClient(
    std::shared_ptr<MqttClient> mqttClient, const SensorType type,
    size_t maxCacheSize)
    : SensorClient(std::move(mqttClient), type),
      cameraInitialized(false),
      imageTopic(IMAGE_TOPIC),
      captureCount(0),
      errorCount(0),
      maxCacheSize(maxCacheSize) {
    imageCache.reserve(maxCacheSize);
    ESP_LOGI(TAG, "Base camera sensor client created with max cache size: %zu",
             maxCacheSize);
}

float BaseCameraSensorClient::readValue() {
    if (!cameraInitialized) {
        if (!initializeCamera()) {
            ESP_LOGE(TAG, "Failed to initialize camera");
            errorCount++;
            return 0.0f;
        }
    }

    std::vector<uint8_t> imageData;
    if (!captureImage(imageData)) {
        ESP_LOGE(TAG, "Failed to capture image");
        errorCount++;
        return 0.0f;
    }

    ESP_LOGI(TAG, "Image captured: %zu bytes", imageData.size());

    imageCache.push_back(std::move(imageData));
    captureCount++;

    if (imageCache.size() >= maxCacheSize) {
        ESP_LOGW(TAG, "Image cache full (%zu images), auto-flushing",
                 imageCache.size());
        flushCache();
    }

    uint32_t total = captureCount + errorCount;
    float successRate =
        total > 0 ? (float)captureCount / total * 100.0f : 100.0f;
    return successRate;
}

void BaseCameraSensorClient::flushImages() {
    if (imageCache.empty()) {
        ESP_LOGD(TAG, "No images to flush");
        return;
    }

    ESP_LOGI(TAG, "Flushing %zu cached images (chunked MQTT)",
             imageCache.size());

    const size_t CHUNK_SIZE =
        4096;  // maks. rozmiar pojedynczej wiadomości MQTT
    bool allSuccess = true;

    for (size_t imgIndex = 0; imgIndex < imageCache.size(); ++imgIndex) {
        const auto& image = imageCache[imgIndex];
        const size_t totalSize = image.size();
        const size_t totalParts = (totalSize + CHUNK_SIZE - 1) / CHUNK_SIZE;

        // unikalny identyfikator ramki
        std::string frameId = std::to_string(imgIndex);
        ESP_LOGI(TAG, "Publishing image %zu (%zu bytes) as %zu parts", imgIndex,
                 totalSize, totalParts);

        for (size_t part = 0; part < totalParts; ++part) {
            size_t offset = part * CHUNK_SIZE;
            size_t len = std::min(CHUNK_SIZE, totalSize - offset);

            std::string topic =
                imageTopic + "/" + frameId + "/part/" + std::to_string(part);
            std::vector<uint8_t> chunk(image.begin() + offset,
                                       image.begin() + offset + len);

            if (!mqtt->publishRaw(topic, chunk)) {
                ESP_LOGE(TAG, "✗ Failed to publish part %zu/%zu of image %zu",
                         part + 1, totalParts, imgIndex);
                allSuccess = false;
                break;
            }

            ESP_LOGD(TAG, "✓ Published part %zu/%zu (%zu bytes)", part + 1,
                     totalParts, len);
            vTaskDelay(
                pdMS_TO_TICKS(50));  // mała przerwa, by uniknąć zalania MQTT
        }

        // sygnał zakończenia ramki
        std::string endTopic = imageTopic + "/" + frameId + "/end";
        std::string endMsg = "frame_end";
        mqtt->publishRaw(endTopic,
                         std::vector<uint8_t>(endMsg.begin(), endMsg.end()));
    }

    if (allSuccess) {
        ESP_LOGI(TAG, "✓ Successfully flushed %zu images (chunked)",
                 imageCache.size());
        imageCache.clear();
    } else {
        ESP_LOGW(TAG, "⚠ Some images failed to publish; keeping in cache");
    }
}

void BaseCameraSensorClient::flushCache() {
    flushImages();
    SensorClient::flushCache();

    ESP_LOGI(TAG,
             "Camera flush complete - Images: flushed, Metrics: sent via "
             "SensorClient");
}