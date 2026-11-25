#include "auth/config/EnvLoader.hpp"

#include <cstdlib>
#include <fstream>
#include <stdexcept>
#include <string>

namespace
{
    std::string trim(const std::string& value)
    {
        const auto first = value.find_first_not_of(" \t\r\n");

        if (first == std::string::npos)
        {
            return {};
        }

        const auto last = value.find_last_not_of(" \t\r\n");

        return value.substr(first, last - first + 1);
    }
}

namespace securecloud::config
{
    void loadEnvFile(const std::filesystem::path& path)
    {
        std::ifstream file(path);

        if (!file.is_open())
        {
            throw std::runtime_error(
                "Impossible d'ouvrir le fichier .env : " +
                path.string()
            );
        }

        std::string line;

        while (std::getline(file, line))
        {
            line = trim(line);

            if (line.empty() || line.starts_with('#'))
            {
                continue;
            }

            const auto separator = line.find('=');

            if (separator == std::string::npos)
            {
                continue;
            }

            std::string key =
                trim(line.substr(0, separator));

            std::string value =
                trim(line.substr(separator + 1));

            if (key.empty())
            {
                continue;
            }

            // Retire les guillemets éventuels
            if (value.size() >= 2)
            {
                if ((value.front() == '"' && value.back() == '"') ||
                    (value.front() == '\'' && value.back() == '\''))
                {
                    value = value.substr(1, value.size() - 2);
                }
            }

            // Une variable système déjà définie reste prioritaire.
            if (std::getenv(key.c_str()) != nullptr)
            {
                continue;
            }

#ifdef _WIN32
            _putenv_s(key.c_str(), value.c_str());
#else
            setenv(key.c_str(), value.c_str(), 1);
#endif
        }
    }
}