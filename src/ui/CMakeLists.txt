# message(STATUS "--- UI MODULE DIAGNOSTICS ---")
# message(STATUS "Root Source Dir: ${CMAKE_SOURCE_DIR}")
# message(STATUS "Looking for includes in: ${CMAKE_SOURCE_DIR}/src")


add_library(ui window_manager.cpp)

target_include_directories(ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/include
)

# # get ui/window libs
# find_package(glfw3 CONFIG REQUIRED)
# find_package(imgui CONFIG REQUIRED)
# find_package(spdlog CONFIG REQUIRED)

# link
target_link_directories(ui
    PUBLIC
        core
        glfw
        imgui::imgui
        OpenGL::GL
        #spdlog::spdlog
)