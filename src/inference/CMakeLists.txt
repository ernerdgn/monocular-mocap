add_library(inference STATIC
    i_inference_backend.hpp
    onnx_cpu_backend.hpp
    onnx_cpu_backend.cpp
)

target_include_directories(inference PUBLIC .)

target_link_libraries(inference 
    PUBLIC 
        core
        onnxruntime_manual
    PRIVATE 
        mocap_warnings
        mocap_sanitizers
)