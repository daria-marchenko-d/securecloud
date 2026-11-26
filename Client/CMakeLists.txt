cmake_minimum_required(VERSION 3.20)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
    Network
)

qt_standard_project_setup()

add_executable(securecloud_client
    src/main.cpp
)

target_link_libraries(securecloud_client
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Network
)