add_library(ui window_manager.cpp)

target_include_directories(ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/include
)

# link
target_link_directories(ui
    PUBLIC
        core
        glfw
        imgui::imgui
        OpenGL::GL
        glad::glad
    PRIVATE
        mocap_warnings
        mocap_sanitizers
)