#pragma once

#include <stdexcept>
#include <string>

enum class Role {
    User,
    Admin
};

inline std::string roleToString(Role role)
{
    return role == Role::Admin ? "ADMIN" : "USER";
}

inline Role roleFromString(const std::string& text)
{
    if (text == "ADMIN") {
        return Role::Admin;
    }
    if (text == "USER") {
        return Role::User;
    }
    throw std::invalid_argument("Unknown role: " + text);
}
