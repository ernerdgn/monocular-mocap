# core module is a header-only interface lib
add_library(core INTERFACE)

# headers
target_include_directories(core INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/src
)

# link
find_package(glm CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

target_link_libraries(core INTERFACE
    glm::glm
    spdlog::spdlog    
)