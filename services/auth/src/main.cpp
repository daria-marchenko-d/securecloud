#include "auth/config/EnvLoader.hpp"
#include "auth/database/Database.hpp"
#include "auth/repositories/UserRepository.hpp"
#include "auth/security/PasswordHasher.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

int main()
{
    try
    {
        // Charge les variables du fichier .env
        securecloud::config::loadEnvFile(".env");

        // Initialisation de la sécurité
        securecloud::security::PasswordHasher::initialize();

        // Connexion PostgreSQL
        Database database;

        if (!database.isConnected())
        {
            std::cerr
                << "Connexion PostgreSQL impossible."
                << std::endl;

            return 1;
        }

        if (!database.ping())
        {
            std::cerr
                << "PostgreSQL connecte mais SELECT 1 a echoue."
                << std::endl;

            return 1;
        }

        std::cout
            << "SecureCloud Auth Service : PostgreSQL OK."
            << std::endl;

        // Repository utilisateur
        securecloud::repositories::UserRepository userRepository(
            database
        );

        // Informations du premier administrateur
        const char* adminEmail =
            std::getenv("SECURECLOUD_ADMIN_EMAIL");

        const char* adminPassword =
            std::getenv("SECURECLOUD_ADMIN_PASSWORD");

        const char* adminFirstName =
            std::getenv("SECURECLOUD_ADMIN_FIRST_NAME");

        const char* adminLastName =
            std::getenv("SECURECLOUD_ADMIN_LAST_NAME");

        if (
            !adminEmail ||
            !adminPassword ||
            !adminFirstName ||
            !adminLastName
        )
        {
            throw std::runtime_error(
                "Configuration du compte administrateur manquante."
            );
        }

        // Vérifie si le compte existe déjà
        const auto existingUser =
            userRepository.findByEmail(adminEmail);

        if (existingUser.has_value())
        {
            std::cout
                << "Le compte administrateur existe deja."
                << std::endl;

            return 0;
        }

        // Hash Argon2id du mot de passe
        const std::string passwordHash =
            securecloud::security::PasswordHasher::hash(
                adminPassword
            );

        // Création utilisateur
        const long long userId =
            userRepository.create(
                adminEmail,
                passwordHash,
                adminFirstName,
                adminLastName
            );

        // Attribution du rôle admin
        userRepository.assignRole(
            userId,
            "admin"
        );

        std::cout
            << "Premier compte administrateur SecureCloud cree."
            << std::endl;

        std::cout
            << "ID utilisateur : "
            << userId
            << std::endl;

        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr
            << "Erreur Auth Service : "
            << e.what()
            << std::endl;

        return 1;
    }
}