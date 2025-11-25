cmake_minimum_required(VERSION 3.10)
project(JuniorJobBoardBackend)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies
find_package(PkgConfig REQUIRED)
pkg_check_modules(MONGOCXX REQUIRED libmongocxx)
pkg_check_modules(BSONCXX REQUIRED libbsoncxx)

find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

# Include directories
include_directories(
    ${MONGOCXX_INCLUDE_DIRS}
    ${BSONCXX_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    .
)

# Server executable
add_executable(server server.cpp)

# Link libraries
target_link_libraries(server
    ${MONGOCXX_LIBRARIES}
    ${BSONCXX_LIBRARIES}
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
)

# Test executable
add_executable(test_application tests/test_application.cpp)
target_link_libraries(test_application
    ${MONGOCXX_LIBRARIES}
    ${BSONCXX_LIBRARIES}
)
