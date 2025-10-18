#pragma once

#include <vector>

#include "../sensor_client.hpp"

class BaseCameraSensorClient : public SensorClient {
   public:
    BaseCameraSensorClient(std::shared_ptr<MqttClient> mqttClient,
                           const SensorType type, size_t maxCacheSize = 10);
    virtual ~BaseCameraSensorClient() = default;

    void flushCache() override;

   protected:
    float readValue() override final;

    virtual bool captureImage(std::vector<uint8_t>& outImage) = 0;
    virtual bool initializeCamera() = 0;
    virtual void cleanupCamera() = 0;

    bool isCameraInitialized() const { return cameraInitialized; }
    void setCameraInitialized(bool initialized) {
        cameraInitialized = initialized;
    }

   private:
    void flushImages();
    // void updateMetricsCache();

    bool cameraInitialized;
    std::string imageTopic;
    std::vector<std::vector<uint8_t>> imageCache;

    uint32_t captureCount;
    uint32_t errorCount;
    size_t maxCacheSize;
};