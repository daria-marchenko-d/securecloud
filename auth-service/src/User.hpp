#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "Role.hpp"

enum class UserStatus {
    Online,
    Away,
    Busy,
    Offline
};

enum class StatusVisibility {
    Everyone,
    Contacts,
    Nobody
};

std::string userStatusToString(UserStatus status);
UserStatus userStatusFromString(const std::string& text);
std::string statusVisibilityToString(StatusVisibility visibility);
StatusVisibility statusVisibilityFromString(const std::string& text);

class User {
public:
    User(std::int64_t identifier,
         std::string email,
         std::string passwordHash,
         Role role,
         std::string name,
         UserStatus status = UserStatus::Offline,
         StatusVisibility statusVisibility = StatusVisibility::Contacts,
         bool isBlocked = false,
         int tokenVersion = 1);

    virtual ~User() = default;

    bool verifyPassword(const std::string& password) const;
    bool changePassword(const std::string& oldPassword, const std::string& newPassword);
    void updateStatus(UserStatus newStatus);
    void incrementTokenVersion();

    std::int64_t getIdentifier() const { return identifier; }
    const std::string& getEmail() const { return email; }
    const std::string& getPasswordHash() const { return passwordHash; }
    Role getRole() const { return role; }
    const std::string& getName() const { return name; }
    UserStatus getStatus() const { return status; }
    StatusVisibility getStatusVisibility() const { return statusVisibility; }
    bool getIsBlocked() const { return isBlocked; }
    int getTokenVersion() const { return tokenVersion; }

    // No setter for passwordHash or role: use changePassword / the Admin class.
    void setIdentifier(std::int64_t newIdentifier) { identifier = newIdentifier; }
    void setStatusVisibility(StatusVisibility newVisibility) { statusVisibility = newVisibility; }
    void setIsBlocked(bool newIsBlocked) { isBlocked = newIsBlocked; }

    static constexpr std::size_t minimumPasswordLength = 12;

private:
    std::int64_t identifier = 0;  // 0 = not saved in the database yet
    std::string email;
    std::string passwordHash;     // Argon2id hash, never the plain password
    Role role = Role::User;
    std::string name;
    UserStatus status = UserStatus::Offline;
    StatusVisibility statusVisibility = StatusVisibility::Contacts;
    bool isBlocked = false;
    int tokenVersion = 1;         // tokens with an older version are rejected
};

class Admin : public User {
public:
    Admin(std::int64_t identifier,
          std::string email,
          std::string passwordHash,
          std::string name,
          UserStatus status = UserStatus::Offline,
          StatusVisibility statusVisibility = StatusVisibility::Contacts,
          bool isBlocked = false,
          int tokenVersion = 1);

    void blockUser(User& targetUser) const;
    void unblockUser(User& targetUser) const;
    void revokeAccess(User& targetUser) const;
};
