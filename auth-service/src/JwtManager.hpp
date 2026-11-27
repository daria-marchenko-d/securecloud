#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>

#include "Role.hpp"
#include "User.hpp"

struct TokenClaims {
    std::int64_t userIdentifier = 0;
    Role role = Role::User;
    int tokenVersion = 0;
};

// HS256 JSON Web Tokens via jwt-cpp.
class JwtManager {
public:
    explicit JwtManager(std::string secretKey,
                        std::chrono::seconds tokenLifetime = std::chrono::hours(1));

    std::string createToken(const User& user) const;

    // std::nullopt = bad signature, expired, wrong issuer or malformed token.
    // The token version is compared with the database by AuthService.
    std::optional<TokenClaims> verifyToken(const std::string& token) const;

    static constexpr std::size_t minimumSecretKeyLength = 32;

private:
    std::string secretKey;
    std::chrono::seconds tokenLifetime;
};
