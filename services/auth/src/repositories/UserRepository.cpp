#include "auth/repositories/UserRepository.hpp"

#include <pqxx/pqxx>

namespace securecloud::repositories
{
    UserRepository::UserRepository(Database& database)
        : database_(database)
    {
    }

    std::optional<models::User>
    UserRepository::findByEmail(const std::string& email)
    {
        pqxx::read_transaction transaction(
            database_.connection()
        );

        const pqxx::result result =
            transaction.exec(
                R"(
                    SELECT
                        id,
                        email,
                        password_hash,
                        first_name,
                        last_name,
                        is_active
                    FROM users
                    WHERE email = $1
                )",
                pqxx::params{
                    email
                }
            );

        if (result.empty())
        {
            return std::nullopt;
        }

        const pqxx::row row = result[0];

        models::User user;

        user.id =
            row["id"].as<long long>();

        user.email =
            row["email"].as<std::string>();

        user.passwordHash =
            row["password_hash"].as<std::string>();

        user.firstName =
            row["first_name"].as<std::string>();

        user.lastName =
            row["last_name"].as<std::string>();

        user.isActive =
            row["is_active"].as<bool>();

        return user;
    }

    long long UserRepository::create(
        const std::string& email,
        const std::string& passwordHash,
        const std::string& firstName,
        const std::string& lastName
    )
    {
        pqxx::work transaction(
            database_.connection()
        );

        const pqxx::result result =
            transaction.exec(
                R"(
                    INSERT INTO users
                    (
                        email,
                        password_hash,
                        first_name,
                        last_name
                    )
                    VALUES ($1, $2, $3, $4)
                    RETURNING id
                )",
                pqxx::params{
                    email,
                    passwordHash,
                    firstName,
                    lastName
                }
            );

        const long long userId =
            result[0]["id"].as<long long>();

        transaction.commit();

        return userId;
    }

    void UserRepository::assignRole(
        long long userId,
        const std::string& roleName
    )
    {
        pqxx::work transaction(
            database_.connection()
        );

        transaction.exec(
            R"(
                INSERT INTO user_roles
                (
                    user_id,
                    role_id
                )
                SELECT $1, id
                FROM roles
                WHERE name = $2
                ON CONFLICT DO NOTHING
            )",
            pqxx::params{
                userId,
                roleName
            }
        );

        transaction.commit();
    }
}