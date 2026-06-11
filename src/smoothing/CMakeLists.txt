add_library(smoothing STATIC
    temporal_smoother.cpp
    temporal_smoother.hpp
    one_euro_filter.hpp
    smoothing_thread.hpp
    smoothing_thread.cpp

)

target_include_directories(smoothing 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(smoothing 
    PUBLIC 
        core
)