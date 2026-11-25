find_package(libpqxx CONFIG REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(
    SODIUM
    REQUIRED
    IMPORTED_TARGET
    libsodium
)

add_executable(auth_service
    src/main.cpp
    src/config/EnvLoader.cpp
    src/database/Database.cpp
    src/security/PasswordHasher.cpp
    src/repositories/UserRepository.cpp
)

target_include_directories(auth_service
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(auth_service
    PRIVATE
        libpqxx::pqxx
        PkgConfig::SODIUM
)