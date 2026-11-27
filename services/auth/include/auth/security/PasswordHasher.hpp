#pragma once

#include <string>

namespace securecloud::security
{
    class PasswordHasher
    {
    public:
        static void initialize();

        static std::string hash(
            const std::string& password
        );

        static bool verify(
            const std::string& password,
            const std::string& PasswordHasher
        );
    };
}