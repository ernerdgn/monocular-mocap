add_library(flow
    optical_flow_processor.cpp
)

target_include_directories(flow 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(flow
    PUBLIC
        core
    PRIVATE
        opencv_core
        opencv_imgproc
        opencv_video
)