#pragma once

#include <string>

// Argon2id via libsodium.
class PasswordHasher {
public:
    static std::string hash(const std::string& password);
    static bool verify(const std::string& hash, const std::string& password);
};
