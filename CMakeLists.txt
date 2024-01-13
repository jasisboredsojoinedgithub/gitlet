cmake_minimum_required(VERSION 3.10)

# Set the project name and version
project(Gitlet VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find the Boost and OpenSSL libraries
find_package(Boost 1.65 REQUIRED COMPONENTS serialization)
find_package(OpenSSL REQUIRED)

# Add the Gitlet executable
add_executable(Gitlet
    src/main.cpp
    src/Commit.cpp
    src/Repo.cpp
    src/StagingArea.cpp
    src/Utils.cpp
)

# Include directories for Gitlet executable
target_include_directories(Gitlet PUBLIC
    ${Boost_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    "${PROJECT_BINARY_DIR}"
    "${PROJECT_SOURCE_DIR}/src"
)

# Link Boost and OpenSSL libraries to the Gitlet executable
target_link_libraries(Gitlet
    ${Boost_LIBRARIES}
    ${OPENSSL_LIBRARIES}
)
