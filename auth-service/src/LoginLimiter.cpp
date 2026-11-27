#include "LoginLimiter.hpp"

LoginLimiter::LoginLimiter(int maximumFailedAttempts,
                           std::chrono::minutes attemptWindow,
                           std::chrono::minutes blockDuration)
    : maximumFailedAttempts(maximumFailedAttempts),
      attemptWindow(attemptWindow),
      blockDuration(blockDuration)
{
}

bool LoginLimiter::isBlocked(const std::string& email) const
{
    return isBlocked(email, Clock::now());
}

void LoginLimiter::registerFailedAttempt(const std::string& email)
{
    registerFailedAttempt(email, Clock::now());
}

bool LoginLimiter::isBlocked(const std::string& email, Clock::time_point now) const
{
    const std::lock_guard<std::mutex> lock(attemptRecordsMutex);

    const auto found = attemptRecords.find(email);
    if (found == attemptRecords.end()) {
        return false;
    }
    return now < found->second.blockedUntil;
}

void LoginLimiter::registerFailedAttempt(const std::string& email, Clock::time_point now)
{
    const std::lock_guard<std::mutex> lock(attemptRecordsMutex);

    AttemptRecord& record = attemptRecords[email];

    const bool windowExpired = now - record.firstFailedAttemptTime > attemptWindow;
    if (record.failedAttempts == 0 || windowExpired) {
        record.failedAttempts = 0;
        record.firstFailedAttemptTime = now;
    }

    ++record.failedAttempts;

    if (record.failedAttempts >= maximumFailedAttempts) {
        record.blockedUntil = now + blockDuration;
        record.failedAttempts = 0;
    }
}

void LoginLimiter::registerSuccessfulLogin(const std::string& email)
{
    const std::lock_guard<std::mutex> lock(attemptRecordsMutex);
    attemptRecords.erase(email);
}
