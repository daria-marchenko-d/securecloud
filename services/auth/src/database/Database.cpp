#include "auth/database/Database.hpp"
#include <cstdlib>
#include <string>
#include <stdexcept>

Database::Database()
{
    const char* host = std::getenv("SECURECLOUD_DB_HOST");
    const char* port = std::getenv("SECURECLOUD_DB_PORT");
    const char* dbname = std::getenv("SECURECLOUD_DB_NAME");
    const char* user = std::getenv("SECURECLOUD_DB_USER");
    const char* password = std::getenv("SECURECLOUD_DB_PASSWORD");

    if (!host || !port || !dbname || !user || !password )
    {
        throw std::runtime_error(
            "Variable d'environnement  PostgreSQL manquantes"
        );
    }

    const std::string connectionString =
    "host=" + std::string(host) +
    " port=" + std::string(port) +
    " dbname=" + std::string(dbname) +
    " user=" + std::string(user) +
    " password=" + std::string(password);

    connection_ =

        std::make_unique<pqxx::connection>(connectionString);
}

bool Database::isConnected() const
{
    return connection_ != nullptr && connection_ -> is_open();
}

bool Database::ping()
{
    if (!isConnected())
    {
        return false;
    }

    pqxx::read_transaction transaction(*connection_);

    const pqxx::result result =
        transaction.exec("SELECT 1");

    return !result.empty() && result[0][0].as<int>() ==1;
    
}

pqxx::connection& Database::connection()
{
    if (!connection_ || !connection_->is_open())
    {
        throw std::runtime_error(
            "Connexion PostgreSQL indisponible."
        );
    }

    return *connection_;
}
