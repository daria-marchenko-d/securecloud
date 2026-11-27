#include "auth/security/PasswordHasher.hpp"

#include <sodium.h>

#include <stdexcept>
#include <string>

namespace securecloud::security
{
    void PasswordHasher::initialize()
    {
        if (sodium_init() < 0)
        {
            throw std::runtime_error(
                "Impossible d'initialiser libsodium."
            );
        }
    }

    std::string PasswordHasher::hash(
        const std::string& password
    )
    {
        if (password.empty())
        {
            throw std::invalid_argument(
                "Le mot de passe ne peut pas etre vide."
            );
        }

        char hash[crypto_pwhash_STRBYTES];

        const int result =
            crypto_pwhash_str_alg(
                hash,
                password.c_str(),
                password.size(),
                crypto_pwhash_OPSLIMIT_INTERACTIVE,
                crypto_pwhash_MEMLIMIT_INTERACTIVE,
                crypto_pwhash_ALG_ARGON2ID13
            );

        if (result != 0)
        {
            throw std::runtime_error(
                "Impossible de hasher le mot de passe."
            );
        }

        return std::string(hash);
    }

    bool PasswordHasher::verify(
        const std::string& password,
        const std::string& passwordHash
    )
    {
        if (password.empty() || passwordHash.empty())
        {
            return false;
        }

        return crypto_pwhash_str_verify(
            passwordHash.c_str(),
            password.c_str(),
            password.size()
        ) == 0;
    }
}