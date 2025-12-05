# Example: How to use finalmq in your CMake project
# After installing finalmq with "sudo make install"

cmake_minimum_required(VERSION 3.11)
project(MyFinalmqApp)

set(CMAKE_CXX_STANDARD 17)

# Method 1: Modern CMake with find_package (Recommended)
find_package(finalmq REQUIRED)

add_executable(myapp main.cpp)

# Link using imported target - automatically handles includes and dependencies
target_link_libraries(myapp PRIVATE finalmq::finalmq)

# That's it! The target finalmq::finalmq provides:
# - Include directories
# - Link libraries
# - Transitive dependencies (pthread, uuid, ssl)

message(STATUS "Found finalmq version: ${finalmq_VERSION}")
