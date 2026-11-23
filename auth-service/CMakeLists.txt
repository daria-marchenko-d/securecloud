cmake_minimum_required(VERSION 3.16)
project(auth-service LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# MSYS2 MinGW64 libraries (libsodium, libpqxx)
list(APPEND CMAKE_PREFIX_PATH "C:/Program Files/msys64/mingw64" "C:/msys64/mingw64")

# libsodium
find_path(SODIUM_INCLUDE_DIR sodium.h)
find_library(SODIUM_LIBRARY NAMES sodium libsodium)
if(NOT SODIUM_INCLUDE_DIR OR NOT SODIUM_LIBRARY)
    message(FATAL_ERROR "libsodium not found. Check CMAKE_PREFIX_PATH and run: pacman -Q mingw-w64-x86_64-libsodium")
endif()

# libpqxx: use its CMake config if available, otherwise find the files manually
find_package(libpqxx CONFIG QUIET)
if(NOT TARGET libpqxx::pqxx)
    find_path(PQXX_INCLUDE_DIR pqxx/pqxx)
    find_library(PQXX_LIBRARY NAMES pqxx)
    find_library(PQ_LIBRARY NAMES pq)
    if(NOT PQXX_INCLUDE_DIR OR NOT PQXX_LIBRARY OR NOT PQ_LIBRARY)
        message(FATAL_ERROR "libpqxx not found. Check CMAKE_PREFIX_PATH and run: pacman -Q mingw-w64-x86_64-libpqxx")
    endif()
    add_library(libpqxx::pqxx INTERFACE IMPORTED)
    set_target_properties(libpqxx::pqxx PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${PQXX_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "${PQXX_LIBRARY};${PQ_LIBRARY}")
    if(WIN32)
        target_link_libraries(libpqxx::pqxx INTERFACE ws2_32)
    endif()
endif()

add_library(auth_deps INTERFACE)
target_include_directories(auth_deps INTERFACE ${SODIUM_INCLUDE_DIR})
target_link_libraries(auth_deps INTERFACE ${SODIUM_LIBRARY} libpqxx::pqxx)

# all sources except main.cpp go into a library, so the tests can reuse them
file(GLOB_RECURSE AUTH_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
set(AUTH_MAIN ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
set(AUTH_LIB_SOURCES ${AUTH_SOURCES})
list(REMOVE_ITEM AUTH_LIB_SOURCES ${AUTH_MAIN})

if(AUTH_LIB_SOURCES)
    add_library(auth_core STATIC ${AUTH_LIB_SOURCES})
    target_include_directories(auth_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_link_libraries(auth_core PUBLIC auth_deps)
endif()

add_executable(auth-service ${AUTH_MAIN})
target_link_libraries(auth-service PRIVATE auth_deps)
if(TARGET auth_core)
    target_link_libraries(auth-service PRIVATE auth_core)
endif()

# tests are built only if tests/ contains .cpp files
file(GLOB TEST_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp)
if(TEST_SOURCES)
    find_package(GTest REQUIRED)
    add_executable(auth-tests ${TEST_SOURCES})
    target_link_libraries(auth-tests PRIVATE GTest::gtest_main auth_deps)
    if(TARGET auth_core)
        target_link_libraries(auth-tests PRIVATE auth_core)
    endif()
    add_test(NAME auth-tests COMMAND auth-tests)
endif()