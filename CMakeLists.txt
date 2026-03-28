cmake_minimum_required(VERSION 3.20)
project(MonocularMoCap CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/utf-8)
    add_compile_options(/wd4459)
endif()

# find pckgs globally
find_package(glm CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenCV CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

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
add_subdirectory(src/ui)
# WILL BE ADDED MORE

add_executable(MonocularMoCap src/main.cpp)

# link
target_link_libraries(MonocularMoCap PRIVATE 
    core 
    capture 
    ui 
    glfw 
    imgui::imgui
    OpenGL::GL
)