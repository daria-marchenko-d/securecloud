// #include "User.hpp"
// #include "PasswordHasher.hpp"
// #include <utility>

// User::User(std::int64_t id, std::string email, std::string passwordHash,
//            Role role, std::string name)
//     : id(id), email(std::move(email)), passwordHash(std::move(passwordHash)),
//       role(role), name(std::move(name)) {}

// bool User::verifyPassword(const std::string& password) const {
//     return PasswordHasher::verify(password, passwordHash);
// }

// bool User::changePassword(const std::string& oldPassword, const std::string& newPassword) {
//     if (!verifyPassword(oldPassword) || newPassword.size() < 12) return false;
//     passwordHash = PasswordHasher::hash(newPassword);
//     return true;
// }

// bool User::login(const std::string& email, const std::string& password) const {
//     return email == this->email && verifyPassword(password);
// }

// void User::updateStatus(UserStatus newStatus) {
//     status = newStatus;
// }

#include "User.hpp"

#include "PasswordHasher.hpp"

#include <stdexcept>
#include <utility>

std::string userStatusToString(UserStatus status)
{
    switch (status) {
    case UserStatus::Online:
        return "ONLINE";
    case UserStatus::Away:
        return "AWAY";
    case UserStatus::Busy:
        return "BUSY";
    case UserStatus::Offline:
        return "OFFLINE";
    }
    throw std::invalid_argument("Unknown user status");
}

UserStatus userStatusFromString(const std::string& text)
{
    if (text == "ONLINE") {
        return UserStatus::Online;
    }
    if (text == "AWAY") {
        return UserStatus::Away;
    }
    if (text == "BUSY") {
        return UserStatus::Busy;
    }
    if (text == "OFFLINE") {
        return UserStatus::Offline;
    }
    throw std::invalid_argument("Unknown user status: " + text);
}

std::string statusVisibilityToString(StatusVisibility visibility)
{
    switch (visibility) {
    case StatusVisibility::Everyone:
        return "EVERYONE";
    case StatusVisibility::Contacts:
        return "CONTACTS";
    case StatusVisibility::Nobody:
        return "NOBODY";
    }
    throw std::invalid_argument("Unknown status visibility");
}

StatusVisibility statusVisibilityFromString(const std::string& text)
{
    if (text == "EVERYONE") {
        return StatusVisibility::Everyone;
    }
    if (text == "CONTACTS") {
        return StatusVisibility::Contacts;
    }
    if (text == "NOBODY") {
        return StatusVisibility::Nobody;
    }
    throw std::invalid_argument("Unknown status visibility: " + text);
}

User::User(std::int64_t identifier,
           std::string email,
           std::string passwordHash,
           Role role,
           std::string name,
           UserStatus status,
           StatusVisibility statusVisibility,
           bool isBlocked,
           int tokenVersion)
    : identifier(identifier),
      email(std::move(email)),
      passwordHash(std::move(passwordHash)),
      role(role),
      name(std::move(name)),
      status(status),
      statusVisibility(statusVisibility),
      isBlocked(isBlocked),
      tokenVersion(tokenVersion)
{
}

bool User::verifyPassword(const std::string& password) const
{
    return PasswordHasher::verify(passwordHash, password);
}

bool User::changePassword(const std::string& oldPassword, const std::string& newPassword)
{
    if (!verifyPassword(oldPassword)) {
        return false;
    }
    if (newPassword.size() < minimumPasswordLength) {
        return false;
    }

    passwordHash = PasswordHasher::hash(newPassword);
    incrementTokenVersion();  // log out every existing session
    return true;
}

void User::updateStatus(UserStatus newStatus)
{
    status = newStatus;
}

void User::incrementTokenVersion()
{
    ++tokenVersion;
}

Admin::Admin(std::int64_t identifier,
             std::string email,
             std::string passwordHash,
             std::string name,
             UserStatus status,
             StatusVisibility statusVisibility,
             bool isBlocked,
             int tokenVersion)
    : User(identifier,
           std::move(email),
           std::move(passwordHash),
           Role::Admin,
           std::move(name),
           status,
           statusVisibility,
           isBlocked,
           tokenVersion)
{
}

void Admin::blockUser(User& targetUser) const
{
    targetUser.setIsBlocked(true);
    targetUser.incrementTokenVersion();
}

void Admin::unblockUser(User& targetUser) const
{
    targetUser.setIsBlocked(false);
}

void Admin::revokeAccess(User& targetUser) const
{
    targetUser.incrementTokenVersion();
}
