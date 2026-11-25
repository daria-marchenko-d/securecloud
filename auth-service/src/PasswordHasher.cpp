// #include "PasswordHasher.h"

// #include <sodium.h>
// #include <stdexcept>

// namespace {

// void ensureSodiumReady() {
//     if (sodium_init() < 0) {
//         throw std::runtime_error("libsodium init failed");
//     }
// }

// }

// std::string PasswordHasher::hash(const std::string& password) {
//     ensureSodiumReady();

//     char hashed[crypto_pwhash_STRBYTES];
//     if (crypto_pwhash_str(hashed, password.c_str(), password.size(),
//                           crypto_pwhash_OPSLIMIT_INTERACTIVE,
//                           crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
//         throw std::runtime_error("password hashing failed");
//     }
//     return std::string(hashed);
// }

// bool PasswordHasher::verify(const std::string& hash, const std::string& password) {
//     ensureSodiumReady();

//     return crypto_pwhash_str_verify(hash.c_str(), password.c_str(), password.size()) == 0;
// }

#include "PasswordHasher.hpp"
 
#include <sodium.h>
#include <stdexcept>
 
namespace {
 
void ensureSodiumReady() {
    if (sodium_init() < 0) {
        throw std::runtime_error("libsodium init failed");
    }
}
 
}
 
std::string PasswordHasher::hash(const std::string& password) {
    ensureSodiumReady();
 
    char hashed[crypto_pwhash_STRBYTES];
    if (crypto_pwhash_str(hashed, password.c_str(), password.size(),
                          crypto_pwhash_OPSLIMIT_INTERACTIVE,
                          crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
        throw std::runtime_error("password hashing failed");
    }
    return std::string(hashed);
}
 
bool PasswordHasher::verify(const std::string& hash, const std::string& password) {
    ensureSodiumReady();
 
    return crypto_pwhash_str_verify(hash.c_str(), password.c_str(), password.size()) == 0;
}