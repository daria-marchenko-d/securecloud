cmake_minimum_required(VERSION 3.20)

project(SecureCloud
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_CLIENT "Build SecureCloud Qt client" ON)
option(BUILD_GATEWAY "Build API Gateway" ON)
option(BUILD_AUTH_SERVICE "Build Auth Service" ON)

if(BUILD_CLIENT)
    add_subdirectory(Client)
endif()

if(BUILD_GATEWAY)
    add_subdirectory(gateway)
endif()

if(BUILD_AUTH_SERVICE)
    add_subdirectory(services/auth)
endif()