cmake_minimum_required(VERSION 3.10)
project(VCS VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# Find required packages
find_package(OpenSSL REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)

# Add include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/core
    ${PROJECT_SOURCE_DIR}/include/models
    ${PROJECT_SOURCE_DIR}/include/utils
    ${OPENSSL_INCLUDE_DIR}
)

# Collect source files
file(GLOB_RECURSE SOURCES
    "src/*.cpp"
)

# Create executable
add_executable(vcs ${SOURCES})

# Link libraries
target_link_libraries(vcs
    PRIVATE
    OpenSSL::SSL
    OpenSSL::Crypto
    nlohmann_json::nlohmann_json
    stdc++fs
)

# Installation
install(TARGETS vcs
    RUNTIME DESTINATION /usr/local/bin
)

# Update or upgrade the VCS in the terminal
install(CODE "
    execute_process(COMMAND sudo ln -sf ${CMAKE_INSTALL_PREFIX}/bin/vcs /usr/bin/vcs
    RESULT_VARIABLE RESULT
    ERROR_VARIABLE ERROR)
    if(NOT RESULT EQUAL 0)
        message(FATAL_ERROR \"Failed to create symlink: ${ERROR}\")
    endif()
")