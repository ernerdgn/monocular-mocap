cmake_minimum_required(VERSION 3.20)
set(VCPKG_LIBRARY_LINKAGE "dynamic")
project(MonocularMoCap CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# add cmake module
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# options
option(MOCAP_ENABLE_SANITIZERS  "Enable ASan + UBSan in Debug builds (non-MSVC)" ON)
option(MOCAP_BUILD_TESTS        "Build the test suite"                            ON)

# global msvc specific
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
find_package(glad CONFIG REQUIRED)
find_package(nfd CONFIG REQUIRED)
# --- MANUAL ONNX RUNTIME SETUP ---
set(ONNX_DIR "D:/software/onnxruntime-win-x64-1.17.1")

add_library(onnx_manual UNKNOWN IMPORTED)
set_target_properties(onnx_manual PROPERTIES
    IMPORTED_LOCATION "${ONNX_DIR}/lib/onnxruntime.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${ONNX_DIR}/include"
)
# ---------------------------------

# sub dirs
add_subdirectory(src/core)
add_subdirectory(src/capture)
add_subdirectory(src/ui)
add_subdirectory(src/detection)

# main exec
add_executable(MonocularMoCap src/main.cpp)
target_link_libraries(MonocularMoCap PRIVATE 
    core 
    capture 
    ui 
    detection
    glfw
    imgui::imgui
    OpenGL::GL
    glad::glad
    nfd::nfd
    onnx_manual
)

# if(MSVC)
#     # 1. OPT:NOREF stops the linker from deleting "unused" code
#     # 2. INCLUDE forces the linker to pull in the ONNX entry point
#     target_link_options(MonocularMoCap PRIVATE "/OPT:NOREF" "/INCLUDE:OrtGetApiBase")
# endif()

# tests
if(MOCAP_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# ==========================================
# CLANG TOOLING TARGETS
# ==========================================

# gather all cpp sources and headers
file(GLOB_RECURSE ALL_CXX_SOURCE_FILES 
    "${CMAKE_SOURCE_DIR}/src/*.cpp" 
    "${CMAKE_SOURCE_DIR}/src/*.hpp"
)

# setup clang format target
find_program(CLANG_FORMAT_EXE NAMES clang-format clang-format-17 clang-format-16)
if(CLANG_FORMAT_EXE)
    add_custom_target(format
        COMMAND ${CLANG_FORMAT_EXE} -i -style=file ${ALL_CXX_SOURCE_FILES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running clang-format to format source files..."
    )
else()
    message(WARNING "clang-format not found. 'format' target will not be created.")
endif()

# setup clang tidy target
find_program(CLANG_TIDY_EXE NAMES clang-tidy clang-tidy-17 clang-tidy-16)
if(CLANG_TIDY_EXE)
    add_custom_target(tidy
        COMMAND ${CLANG_TIDY_EXE} -p ${CMAKE_BINARY_DIR} ${ALL_CXX_SOURCE_FILES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running clang-tidy to check for warnings..."
    )
else()
    message(WARNING "clang-tidy not found. 'tidy' target will not be created.")
endif()