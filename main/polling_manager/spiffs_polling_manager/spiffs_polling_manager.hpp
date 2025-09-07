#pragma once

#include "polling_manager/polling_manager.hpp"
#include "utils/file_manager/file_manager.hpp"

class SpiffsPollingManager final : public PollingManager {
   public:
    using PollingManager::PollingManager;

    void pollAction() const override;

   private:
    void moveFile(const char* path) const;
};
