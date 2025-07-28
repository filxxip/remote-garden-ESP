#pragma once

#include <cstdint>
#include <functional>

struct GattCharacteristicDef {
    uint16_t uuid16;
    size_t bufferSize;
    const char* label;
    std::function<void(const uint8_t* data, uint16_t length)> onWrite;
};
