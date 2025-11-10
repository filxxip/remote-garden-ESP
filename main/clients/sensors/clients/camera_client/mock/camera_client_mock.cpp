#include "camera_client_mock.hpp"

#include <esp_log.h>

namespace {
constexpr const char* TAG = "MockCameraSensorClient";

constexpr size_t MIN_IMAGE_SIZE = 1024;
constexpr size_t MAX_IMAGE_SIZE = 5120;

const uint8_t JPEG_HEADER[] = {0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x4A,
                               0x46, 0x49, 0x46, 0x00, 0x01, 0x01, 0x00,
                               0x00, 0x01, 0x00, 0x01, 0x00, 0x00};

const uint8_t JPEG_FOOTER[] = {0xFF, 0xD9};

constexpr size_t JPEG_HEADER_SIZE = sizeof(JPEG_HEADER);
constexpr size_t JPEG_FOOTER_SIZE = sizeof(JPEG_FOOTER);
}  // namespace

MockCameraSensorClient::MockCameraSensorClient(
    std::shared_ptr<MqttClient> mqttClient, const SensorType type)
    : BaseCameraSensorClient(std::move(mqttClient), type),
      rng(std::random_device{}()) {
    ESP_LOGI(TAG, "[MOCK] Camera sensor client created");
}

void MockCameraSensorClient::cleanupCamera() {
    ESP_LOGI(TAG, "[MOCK] Cleaning up mock camera");
}

bool MockCameraSensorClient::captureImage(std::vector<uint8_t>& outImage) {
    ESP_LOGI(TAG, "[MOCK] Generating mock image");

    std::uniform_int_distribution<size_t> sizeDist(MIN_IMAGE_SIZE,
                                                   MAX_IMAGE_SIZE);
    size_t dataSize = sizeDist(rng);

    outImage.clear();
    outImage.reserve(JPEG_HEADER_SIZE + dataSize + JPEG_FOOTER_SIZE);

    // Add JPEG header
    outImage.insert(outImage.end(), JPEG_HEADER,
                    JPEG_HEADER + JPEG_HEADER_SIZE);

    // Add random data
    std::uniform_int_distribution<uint8_t> byteDist(0, 255);
    for (size_t i = 0; i < dataSize; ++i) {
        outImage.push_back(byteDist(rng));
    }

    // Add JPEG footer
    outImage.insert(outImage.end(), JPEG_FOOTER,
                    JPEG_FOOTER + JPEG_FOOTER_SIZE);

    ESP_LOGI(TAG, "[MOCK] Mock image generated: %zu bytes", outImage.size());
    return true;
}