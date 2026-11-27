// #include <sodium.h>
// #include <pqxx/pqxx>

// #include <cstdlib>
// #include <iostream>
// #include <string>

// namespace {

// std::string envOr(const char* name, const std::string& fallback) {
//     const char* value = std::getenv(name);
//     return (value != nullptr && value[0] != '\0') ? std::string(value) : fallback;
// }

// }

// int main() {
//     std::cout << "[auth-service] starting\n";

//     // returns 0 = ok, 1 = already initialized, -1 = failure
//     if (sodium_init() < 0) {
//         std::cerr << "[auth-service] libsodium init failed\n";
//         return 1;
//     }
//     std::cout << "[auth-service] libsodium OK\n";

//     // the password is read from the PGPASSWORD environment variable
//     const std::string connectionString = envOr(
//         "AUTH_DB_CONNSTR",
//         "host=localhost port=5432 dbname=securecloud user=postgres");

//     try {
//         pqxx::connection connection(connectionString);
//         pqxx::nontransaction tx(connection);
//         const pqxx::result result = tx.exec("SELECT 1");
//         std::cout << "[auth-service] PostgreSQL OK (db: " << connection.dbname()
//                   << ", SELECT 1 = " << result[0][0].as<int>() << ")\n";
//     } catch (const std::exception& error) {
//         std::cerr << "[auth-service] PostgreSQL error: " << error.what() << "\n";
//         return 1;
//     }

//     std::cout << "[auth-service] ready\n";
//     return 0;
// }

#include <sodium.h>
#include <pqxx/pqxx>

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <optional>
#include <string>

#include "JwtManager.hpp"
#include "LoginLimiter.hpp"
#include "PasswordHasher.hpp"
#include "PermissionService.hpp"
#include "User.hpp"
#include "tests/FakeUserRepository.hpp"

namespace {

std::string envOr(const char* name, const std::string& fallback) {
    const char* value = std::getenv(name);
    return (value != nullptr && value[0] != '\0') ? std::string(value) : fallback;
}

void printCheck(const std::string& label, bool passed) {
    std::cout << "[auth-service]   " << (passed ? "OK   " : "FAIL ") << label << "\n";
}

bool runUserSelfCheck() {
    std::cout << "[auth-service] User self-check\n";

    User user(7, "dari@mail.com", PasswordHasher::hash("CorrectPassword123"), Role::User, "Dari");
    Admin admin(1, "admin@mail.com", PasswordHasher::hash("AdminPassword123"), "Admin");

    const bool correctPasswordAccepted = user.verifyPassword("CorrectPassword123");
    const bool wrongPasswordRejected = !user.verifyPassword("WrongPassword123");
    const bool passwordChanged = user.changePassword("CorrectPassword123", "NewPassword12345");
    const bool newPasswordAccepted = user.verifyPassword("NewPassword12345");
    admin.blockUser(user);
    const bool userBlocked = user.getIsBlocked();
    const bool adminHasAdminRole = admin.getRole() == Role::Admin;

    printCheck("correct password accepted", correctPasswordAccepted);
    printCheck("wrong password rejected", wrongPasswordRejected);
    printCheck("password changed", passwordChanged);
    printCheck("new password accepted", newPasswordAccepted);
    printCheck("user blocked by admin", userBlocked);
    printCheck("admin has ADMIN role", adminHasAdminRole);

    return correctPasswordAccepted && wrongPasswordRejected && passwordChanged
        && newPasswordAccepted && userBlocked && adminHasAdminRole;
}


bool runRepositorySelfCheck() {
    std::cout << "[auth-service] Repository self-check\n";

    FakeUserRepository fakeRepository;
    IUserRepository& repository = fakeRepository;

    const std::int64_t userIdentifier = repository.create(
        User(0, "dari@mail.com", PasswordHasher::hash("CorrectPassword123"), Role::User, "Dari"));
    const std::int64_t adminIdentifier = repository.create(
        Admin(0, "admin@mail.com", PasswordHasher::hash("AdminPassword123"), "Admin"));

    const bool identifiersAssigned = userIdentifier == 1 && adminIdentifier == 2;
    const bool emailExists = repository.existsByEmail("dari@mail.com");
    const bool unknownEmailRejected = !repository.existsByEmail("nobody@mail.com");

    std::unique_ptr<User> foundUser = repository.findByEmail("dari@mail.com");
    const bool userFoundByEmail = foundUser != nullptr
        && foundUser->getIdentifier() == userIdentifier
        && foundUser->verifyPassword("CorrectPassword123");

    std::unique_ptr<User> foundAdmin = repository.findByIdentifier(adminIdentifier);
    const Admin* admin = dynamic_cast<const Admin*>(foundAdmin.get());
    const bool adminReturnedAsAdmin = admin != nullptr;

    bool blockSaved = false;
    if (admin != nullptr && foundUser != nullptr) {
        admin->blockUser(*foundUser);
        blockSaved = repository.update(*foundUser)
            && repository.findByIdentifier(userIdentifier)->getIsBlocked();
    }

    const bool unknownIdentifierRejected = repository.findByIdentifier(99) == nullptr;

    printCheck("identifiers 1 and 2 assigned", identifiersAssigned);
    printCheck("existing email found", emailExists);
    printCheck("unknown email not found", unknownEmailRejected);
    printCheck("user found by email", userFoundByEmail);
    printCheck("admin returned as Admin", adminReturnedAsAdmin);
    printCheck("blocking saved by update", blockSaved);
    printCheck("unknown identifier not found", unknownIdentifierRejected);

    return identifiersAssigned && emailExists && unknownEmailRejected && userFoundByEmail
        && adminReturnedAsAdmin && blockSaved && unknownIdentifierRejected;
}


bool runJwtSelfCheck() {
    std::cout << "[auth-service] JWT self-check\n";

    const JwtManager jwtManager("check-only-secret-key-0123456789abcdef");
    const JwtManager otherJwtManager("another-secret-key-0123456789abcdef");
    const JwtManager expiredJwtManager("check-only-secret-key-0123456789abcdef",
                                       std::chrono::seconds(-10));

    const User user(7, "dari@mail.com", PasswordHasher::hash("CorrectPassword123"), Role::User, "Dari");
    const std::string token = jwtManager.createToken(user);

    const std::optional<TokenClaims> claims = jwtManager.verifyToken(token);
    const bool validTokenAccepted = claims.has_value()
        && claims->userIdentifier == 7
        && claims->role == Role::User
        && claims->tokenVersion == 1;

    std::string changedToken = token;
    char& payloadCharacter = changedToken.at(changedToken.find('.') + 5);
    payloadCharacter = payloadCharacter == 'A' ? 'B' : 'A';
    const bool changedTokenRejected = !jwtManager.verifyToken(changedToken).has_value();

    const bool otherKeyRejected = !otherJwtManager.verifyToken(token).has_value();

    const std::string expiredToken = expiredJwtManager.createToken(user);
    const bool expiredTokenRejected = !jwtManager.verifyToken(expiredToken).has_value();

    printCheck("valid token accepted", validTokenAccepted);
    printCheck("changed token rejected", changedTokenRejected);
    printCheck("token from another key rejected", otherKeyRejected);
    printCheck("expired token rejected", expiredTokenRejected);

    return validTokenAccepted && changedTokenRejected && otherKeyRejected && expiredTokenRejected;
}

bool runLoginLimiterSelfCheck() {
    std::cout << "[auth-service] LoginLimiter self-check\n";

    LoginLimiter loginLimiter;
    const LoginLimiter::Clock::time_point start = LoginLimiter::Clock::now();

    for (int attempt = 0; attempt < 5; ++attempt) {
        loginLimiter.registerFailedAttempt("dari@mail.com", start);
    }
    const bool notBlockedAfterFive = !loginLimiter.isBlocked("dari@mail.com", start);

    loginLimiter.registerFailedAttempt("dari@mail.com", start);
    const bool blockedAfterSix = loginLimiter.isBlocked("dari@mail.com", start);

    const bool otherEmailNotBlocked = !loginLimiter.isBlocked("other@mail.com", start);

    const bool unblockedAfterFifteenMinutes =
        !loginLimiter.isBlocked("dari@mail.com", start + std::chrono::minutes(15));

    for (int attempt = 0; attempt < 5; ++attempt) {
        loginLimiter.registerFailedAttempt("slow@mail.com", start);
    }
    loginLimiter.registerFailedAttempt("slow@mail.com", start + std::chrono::minutes(16));
    const bool oldAttemptsForgotten =
        !loginLimiter.isBlocked("slow@mail.com", start + std::chrono::minutes(16));

    printCheck("not blocked after 5 failed attempts", notBlockedAfterFive);
    printCheck("blocked after 6 failed attempts", blockedAfterSix);
    printCheck("other email not blocked", otherEmailNotBlocked);
    printCheck("unblocked after 15 minutes", unblockedAfterFifteenMinutes);
    printCheck("attempts older than 15 minutes forgotten", oldAttemptsForgotten);

    return notBlockedAfterFive && blockedAfterSix && otherEmailNotBlocked
        && unblockedAfterFifteenMinutes && oldAttemptsForgotten;
}

bool runPermissionSelfCheck() {
    std::cout << "[auth-service] PermissionService self-check\n";

    const bool userChangesOwnPassword =
        PermissionService::canActOnUser(Role::User, 7, Permission::ChangeOwnPassword, 7);
    const bool userCannotChangeOtherPassword =
        !PermissionService::canActOnUser(Role::User, 7, Permission::ChangeOwnPassword, 8);
    const bool userCannotBlock =
        !PermissionService::canActOnUser(Role::User, 7, Permission::BlockUser, 8);
    const bool adminBlocksOtherUser =
        PermissionService::canActOnUser(Role::Admin, 1, Permission::BlockUser, 7);
    const bool adminCannotBlockSelf =
        !PermissionService::canActOnUser(Role::Admin, 1, Permission::BlockUser, 1);

    printCheck("user changes own password", userChangesOwnPassword);
    printCheck("user cannot change another password", userCannotChangeOtherPassword);
    printCheck("user cannot block", userCannotBlock);
    printCheck("admin blocks another user", adminBlocksOtherUser);
    printCheck("admin cannot block own account", adminCannotBlockSelf);

    return userChangesOwnPassword && userCannotChangeOtherPassword && userCannotBlock
        && adminBlocksOtherUser && adminCannotBlockSelf;
}

}

int main() {
    std::cout << "[auth-service] starting\n";

    // returns 0 = ok, 1 = already initialized, -1 = failure
    if (sodium_init() < 0) {
        std::cerr << "[auth-service] libsodium init failed\n";
        return 1;
    }
    std::cout << "[auth-service] libsodium OK\n";

    if (!runUserSelfCheck()) {
        std::cerr << "[auth-service] User self-check failed\n";
        return 1;
    }
    std::cout << "[auth-service] User self-check OK\n";

    if (!runRepositorySelfCheck()) {
        std::cerr << "[auth-service] Repository self-check failed\n";
        return 1;
    }
    std::cout << "[auth-service] Repository self-check OK\n";

    if (!runJwtSelfCheck() || !runLoginLimiterSelfCheck() || !runPermissionSelfCheck()) {
        std::cerr << "[auth-service] Security self-check failed\n";
        return 1;
    }
    std::cout << "[auth-service] Security self-check OK\n";

    // the password is read from the PGPASSWORD environment variable
    const std::string connectionString = envOr(
        "AUTH_DB_CONNSTR",
        "host=localhost port=5432 dbname=securecloud user=postgres");

    try {
        pqxx::connection connection(connectionString);
        pqxx::nontransaction tx(connection);
        const pqxx::result result = tx.exec("SELECT 1");
        std::cout << "[auth-service] PostgreSQL OK (db: " << connection.dbname()
                  << ", SELECT 1 = " << result[0][0].as<int>() << ")\n";
    } catch (const std::exception& error) {
        std::cerr << "[auth-service] PostgreSQL error: " << error.what() << "\n";
        return 1;
    }

    std::cout << "[auth-service] ready\n";
    return 0;
}
