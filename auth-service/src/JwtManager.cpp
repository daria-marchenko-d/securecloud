#include "JwtManager.hpp"

#include <jwt-cpp/jwt.h>

#include <stdexcept>
#include <utility>

namespace {

const std::string tokenIssuer = "securecloud-auth-service";
const std::string roleClaimName = "role";
const std::string tokenVersionClaimName = "token_version";

}

JwtManager::JwtManager(std::string secretKey, std::chrono::seconds tokenLifetime)
    : secretKey(std::move(secretKey)),
      tokenLifetime(tokenLifetime)
{
    if (this->secretKey.size() < minimumSecretKeyLength) {
        throw std::invalid_argument("JWT secret key must be at least 32 characters");
    }
}

std::string JwtManager::createToken(const User& user) const
{
    const auto issuedAt = std::chrono::system_clock::now();

    return jwt::create()
        .set_type("JWT")
        .set_issuer(tokenIssuer)
        .set_subject(std::to_string(user.getIdentifier()))
        .set_issued_at(issuedAt)
        .set_expires_at(issuedAt + tokenLifetime)
        .set_payload_claim(roleClaimName, jwt::claim(roleToString(user.getRole())))
        .set_payload_claim(tokenVersionClaimName,
                           jwt::claim(std::to_string(user.getTokenVersion())))
        .sign(jwt::algorithm::hs256{secretKey});
}

std::optional<TokenClaims> JwtManager::verifyToken(const std::string& token) const
{
    try {
        const auto decodedToken = jwt::decode(token);

        jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{secretKey})
            .with_issuer(tokenIssuer)
            .verify(decodedToken);

        TokenClaims claims;
        claims.userIdentifier = std::stoll(decodedToken.get_subject());
        claims.role = roleFromString(decodedToken.get_payload_claim(roleClaimName).as_string());
        claims.tokenVersion =
            std::stoi(decodedToken.get_payload_claim(tokenVersionClaimName).as_string());
        return claims;
    } catch (const std::exception&) {
        return std::nullopt;
    }
}
