cmake_minimum_required(VERSION 3.16)
project(JuniorJobBoardBackend)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependencies installed from APT inside Docker
find_package(mongocxx REQUIRED)
find_package(bsoncxx REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

# Include directories
include_directories(
    ${OPENSSL_INCLUDE_DIR}
    .
    ./config
    ./models
    ./routes
    ./services
    ./utils
    ./controllers
)

# Gather all .cpp files
file(GLOB_RECURSE SOURCES
    server.cpp
    config/*.cpp
    models/*.cpp
    routes/*.cpp
    services/*.cpp
    utils/*.cpp
    controllers/*.cpp
)

# Create executable
add_executable(server ${SOURCES})

# Link libs
target_link_libraries(server
    bsoncxx::bsoncxx_shared
    mongocxx::mongocxx_shared
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
)
