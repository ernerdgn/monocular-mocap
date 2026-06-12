add_library(session STATIC
    session_serializer.cpp
    session_serializer.hpp
)

target_include_directories(session 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(session 
    PUBLIC 
        core
)