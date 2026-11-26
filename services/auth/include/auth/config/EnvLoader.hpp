#pragma once

#include <filesystem>

namespace securecloud::config
{
    void loadEnvFile(const std::filesystem::path& path);
}