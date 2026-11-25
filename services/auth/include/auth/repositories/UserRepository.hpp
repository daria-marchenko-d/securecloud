#pragma once

#include "auth/database/Database.hpp"
#include "auth/models/User.hpp"

#include <string>
#include <optional>

namespace securecloud::repositories
{
    class UserRepository
    {
    public:
        explicit UserRepository(Database& database);

        std::optional<models::User> findByEmail(
            const std::string& email
        );

        long long create(
            const std::string& email,
            const std::string& passwordHash,
            const std::string& firstName,
            const std::string& lastName
        );
        
        void assignRole(
            long long userId,
            const std::string& roleName
        );
    private:
        Database& database_;

    };
}