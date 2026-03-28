cmake_minimum_required(VERSION 3.21)
project(MonocularMoCap LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# warnings
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# unity build for release
set(CMAKE_UNITY_BUILD_BATCH_SIZE 0)
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_UNITY_BUILD ON)
endif()

# module subdirs
add_subdirectory(src/core)
add_subdirectory(src/capture)
# WILL BE ADDED MORE

add_executable(MonocularMoCap src/main.cpp)

# link
target_link_libraries(MonocularMoCap PRIVATE core capture)