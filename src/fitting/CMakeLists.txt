add_library(fitting STATIC
    smplx_model.hpp
    smplx_model.cpp
    pose_optimizer.hpp
    pose_optimizer.cpp
    fitting_thread.hpp
    fitting_thread.cpp
)

target_include_directories(fitting PUBLIC .)

target_link_libraries(fitting 
    PUBLIC 
        core
        inference
        detection
    PRIVATE
        mocap_warnings
        mocap_sanitizers
)