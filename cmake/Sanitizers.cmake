add_library(mocap_sanitizers INTERFACE)

if(MOCAP_ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT MSVC)
    message(STATUS "[mocap] Sanitizers: AddressSanitizer + UndefinedBehaviorSanitizer ENABLED")
    target_compile_options(mocap_sanitizers INTERFACE
        -fsanitize=address,undefined -fno-omit-frame-pointer 
        -fno-optimize-sibling-calls -g
    )
    target_link_options(mocap_sanitizers INTERFACE -fsanitize=address,undefined)
else()
    if(MOCAP_ENABLE_SANITIZERS AND MSVC)
        message(STATUS "[mocap] Sanitizers: skipped (MSVC — use the MSVC debugger instead)")
    elseif(MOCAP_ENABLE_SANITIZERS)
        message(STATUS "[mocap] Sanitizers: skipped (Release build)")
    else()
        message(STATUS "[mocap] Sanitizers: disabled by option")
    endif()
endif()