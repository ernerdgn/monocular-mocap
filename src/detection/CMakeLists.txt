add_library(detection STATIC
    i_detector.hpp
    onnx_detector.hpp
    onnx_detector.cpp
    detection_thread.hpp
    detection_thread.cpp
)

target_include_directories(detection PUBLIC .)

target_link_libraries(detection 
    PUBLIC 
        core
        onnx_manual
        ${OpenCV_LIBS}
    PRIVATE 
        mocap_warnings
        mocap_sanitizers
)