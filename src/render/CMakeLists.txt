add_library(render STATIC
    window_manager.hpp
    window_manager.cpp
    texture.hpp
    texture.cpp
)

target_include_directories(render PUBLIC .)

target_link_libraries(render 
    PUBLIC 
        core
        glfw
        OpenGL::GL
        glad::glad
        ${OpenCV_LIBS}
    PRIVATE 
        mocap_warnings
        mocap_sanitizers
)