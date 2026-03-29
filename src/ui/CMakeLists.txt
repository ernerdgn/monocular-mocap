add_library(ui STATIC
    window_manager.cpp
    texture.cpp
    controls_panel.cpp
    viewport_panel.cpp
    status_panel.cpp
    console_panel.cpp
    main_ui.cpp
)

target_include_directories(ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/vcpkg_installed/x64-windows/include
    ${OpenCV_INCLUDE_DIRS}
)

# link
target_link_directories(ui
    PUBLIC
        core
        glfw
        imgui::imgui
        OpenGL::GL
        glad::glad
        ${OpenCV_LIBS}
        nfd::nfd
    PRIVATE
        mocap_warnings
        mocap_sanitizers
)