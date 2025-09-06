#pragma once

#include <string>

namespace NvsCredentialsManager {

struct Credentials {
    std::string ssid;
    std::string password;
    bool valid() const { return !ssid.empty() && !password.empty(); }
};

bool save(const Credentials& creds);
bool load(Credentials& creds);
bool clear();
};  // namespace NvsCredentialsManager
