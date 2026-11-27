#pragma once

#include <chrono>
#include <mutex>
#include <string>
#include <unordered_map>

// Blocks login for an email after too many failed attempts (brute force protection).
class LoginLimiter {
public:
    using Clock = std::chrono::steady_clock;

    explicit LoginLimiter(int maximumFailedAttempts = 6,
                          std::chrono::minutes attemptWindow = std::chrono::minutes(15),
                          std::chrono::minutes blockDuration = std::chrono::minutes(15));

    bool isBlocked(const std::string& email) const;
    void registerFailedAttempt(const std::string& email);
    void registerSuccessfulLogin(const std::string& email);

    // Same as above with an explicit time, so the logic can be checked without waiting.
    bool isBlocked(const std::string& email, Clock::time_point now) const;
    void registerFailedAttempt(const std::string& email, Clock::time_point now);

private:
    struct AttemptRecord {
        int failedAttempts = 0;
        Clock::time_point firstFailedAttemptTime;
        Clock::time_point blockedUntil;
    };

    int maximumFailedAttempts;
    std::chrono::minutes attemptWindow;
    std::chrono::minutes blockDuration;

    std::unordered_map<std::string, AttemptRecord> attemptRecords;
    mutable std::mutex attemptRecordsMutex;  // requests are handled in parallel threads
};
