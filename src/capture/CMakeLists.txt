# creating cap lib
add_library(capture
    camera_source.cpp
    capture_thread.cpp
    file_source.cpp
    device_enumerator.cpp
)

# allow others to get this dude
target_include_directories(capture PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

# # get cv
# find_package(OpenCV CONFIG REQUIRED)

# link
target_link_libraries(capture 
    PUBLIC 
        core
        ${OpenCV_LIBS}
    PRIVATE
        mocap_warnings
        mocap_sanitizers
)