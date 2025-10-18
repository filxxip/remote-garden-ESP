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

    ESP_LOGI(TAG, "Flushing %zu cached images", imageCache.size());

    bool allSuccess = true;
    for (size_t i = 0; i < imageCache.size(); ++i) {
        const auto& image = imageCache[i];

        std::string payload(reinterpret_cast<const char*>(image.data()),
                            image.size());
        std::string topicWithIndex = imageTopic + "/" + std::to_string(i);

        if (!mqtt->publish(topicWithIndex, payload)) {
            ESP_LOGE(TAG, "Failed to publish image %zu", i);
            allSuccess = false;
        } else {
            ESP_LOGD(TAG, "Published image %zu: %zu bytes", i, image.size());
        }
    }

    if (allSuccess) {
        ESP_LOGI(TAG, "Successfully flushed %zu images", imageCache.size());
        imageCache.clear();
    } else {
        ESP_LOGW(TAG, "Some images failed to publish, keeping in cache");
    }
}

void BaseCameraSensorClient::flushCache() {
    flushImages();
    SensorClient::flushCache();

    ESP_LOGI(TAG,
             "Camera flush complete - Images: flushed, Metrics: sent via "
             "SensorClient");
}