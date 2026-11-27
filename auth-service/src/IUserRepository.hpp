#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "User.hpp"

class IUserRepository {
public:
    virtual ~IUserRepository() = default;

    // nullptr = not found; an ADMIN row is returned as an Admin object
    virtual std::unique_ptr<User> findByIdentifier(std::int64_t identifier) const = 0;
    virtual std::unique_ptr<User> findByEmail(const std::string& email) const = 0;
    virtual bool existsByEmail(const std::string& email) const = 0;

    // returns the identifier assigned by the storage
    virtual std::int64_t create(const User& user) = 0;

    // false = no user with this identifier
    virtual bool update(const User& user) = 0;
};
