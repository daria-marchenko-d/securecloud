#pragma once

#include <string>

namespace securecloud::models
{
    struct User
    {
        long long id{};

        std::string email;
        std::string passwordHash;
        std::string firstName;
        std::string lastName;

        bool isActive{false};
    };
}