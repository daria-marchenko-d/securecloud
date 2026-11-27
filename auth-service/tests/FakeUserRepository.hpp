#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <string>

#include "IUserRepository.hpp"
#include "User.hpp"

// In-memory storage for tests, no database needed.
class FakeUserRepository : public IUserRepository {
public:
    std::unique_ptr<User> findByIdentifier(std::int64_t identifier) const override
    {
        const auto found = users.find(identifier);
        if (found == users.end()) {
            return nullptr;
        }
        return copyOf(found->second);
    }

    std::unique_ptr<User> findByEmail(const std::string& email) const override
    {
        for (const auto& [identifier, user] : users) {
            if (user.getEmail() == email) {
                return copyOf(user);
            }
        }
        return nullptr;
    }

    bool existsByEmail(const std::string& email) const override
    {
        return findByEmail(email) != nullptr;
    }

    std::int64_t create(const User& user) override
    {
        const std::int64_t newIdentifier = nextIdentifier;
        ++nextIdentifier;

        User storedUser = user;
        storedUser.setIdentifier(newIdentifier);
        users.emplace(newIdentifier, storedUser);
        return newIdentifier;
    }

    bool update(const User& user) override
    {
        const auto found = users.find(user.getIdentifier());
        if (found == users.end()) {
            return false;
        }
        found->second = user;
        return true;
    }

private:
    static std::unique_ptr<User> copyOf(const User& user)
    {
        if (user.getRole() == Role::Admin) {
            return std::make_unique<Admin>(user.getIdentifier(),
                                           user.getEmail(),
                                           user.getPasswordHash(),
                                           user.getName(),
                                           user.getStatus(),
                                           user.getStatusVisibility(),
                                           user.getIsBlocked(),
                                           user.getTokenVersion());
        }
        return std::make_unique<User>(user);
    }

    std::map<std::int64_t, User> users;
    std::int64_t nextIdentifier = 1;
};
