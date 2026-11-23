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

#include <cstdlib>
#include <iostream>
#include <string>

#include "PasswordHasher.hpp"
#include "User.hpp"

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