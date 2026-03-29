add_library(core STATIC 
    config.cpp
    logger.cpp
)

target_include_directories(core PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(core 
    PUBLIC 
        glm::glm 
        spdlog::spdlog_header_only
        nlohmann_json::nlohmann_json
    PRIVATE 
        mocap_warnings
        mocap_sanitizers
)