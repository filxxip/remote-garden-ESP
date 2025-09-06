#include "scoped_lock.hpp"

#include <cstddef>

ScopedLock::ScopedLock(SemaphoreHandle_t m) : mutex(m), locked(false) {
    if (mutex) {
        if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
            locked = true;
        }
    }
}

ScopedLock::~ScopedLock() {
    if (locked && mutex) {
        xSemaphoreGive(mutex);
    }
}

ScopedLock::ScopedLock(ScopedLock&& other) noexcept
    : mutex(other.mutex), locked(other.locked) {
    other.mutex = nullptr;
    other.locked = false;
}

ScopedLock& ScopedLock::operator=(ScopedLock&& other) noexcept {
    if (this != &other) {
        if (locked && mutex) {
            xSemaphoreGive(mutex);
        }
        mutex = other.mutex;
        locked = other.locked;
        other.mutex = nullptr;
        other.locked = false;
    }
    return *this;
}
