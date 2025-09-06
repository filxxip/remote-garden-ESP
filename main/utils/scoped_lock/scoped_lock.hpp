#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

class ScopedLock {
   public:
    explicit ScopedLock(SemaphoreHandle_t m);
    ~ScopedLock();

    ScopedLock(const ScopedLock&) = delete;
    ScopedLock& operator=(const ScopedLock&) = delete;

    ScopedLock(ScopedLock&& other) noexcept;
    ScopedLock& operator=(ScopedLock&& other) noexcept;

    bool isLocked() const { return locked; }

   private:
    SemaphoreHandle_t mutex;
    bool locked;
};
