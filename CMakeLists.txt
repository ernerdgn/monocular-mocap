cmake_minimum_required(VERSION 3.20)
#set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL$<$<CONFIG:Debug>:Debug>")
project(MonocularMoCap CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# add cmake module
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# options
option(MOCAP_ENABLE_SANITIZERS  "Enable ASan + UBSan in Debug builds (non-MSVC)" ON)
option(MOCAP_BUILD_TESTS        "Build the test suite"                            ON)

# global msvc spesific
if(MSVC)
    add_compile_options(/utf-8)   
    add_compile_options(/wd4459)  
endif()

# cmake include
include(CompilerWarnings)
include(Sanitizers)

# get packages globally
find_package(glm CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenCV CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

# sub dirs
add_subdirectory(src/core)
add_subdirectory(src/capture)
add_subdirectory(src/ui)

# main exec
add_executable(MonocularMoCap src/main.cpp)
target_link_libraries(MonocularMoCap PRIVATE 
    core 
    capture 
    ui 
    glfw 
    imgui::imgui 
    OpenGL::GL
)

# tests
if(MOCAP_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()