cmake_minimum_required(VERSION 3.20)

add_executable(securecloud_gateway
    src/main.cpp
)

target_include_directories(securecloud_gateway
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)