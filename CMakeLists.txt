cmake_minimum_required(VERSION 3.25)

project(
    magic_builder
    VERSION 1.0.0
    HOMEPAGE_URL "https://github.com/djava/magic_builder"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD_REQUIRED 20)

set(EXPORT_NAMESPACE "magic_bldr::")

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(CMAKE_CXX_STANDARD 20)

    # Disable GNU extensions 
    set(CMAKE_CXX_EXTENSIONS OFF)

    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    set(CMAKE_CXX_COMPILER "clang++")
    set(CMAKE_OUTPUT_DIRECTORY build/bin)
endif()

add_library(magic_builder main.cpp)
target_include_directories(magic_builder PUBLIC include/)

add_executable(magic_builder_test main.cpp)
target_include_directories(magic_builder_test PUBLIC include/)
