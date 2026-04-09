# get catch2 from github to compile with c++20
include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.5.2
)
FetchContent_MakeAvailable(Catch2)

# test sources
add_executable(mocap_tests
    main_test.cpp
    test_concurrent_queue.cpp
    test_result.cpp
    test_detection.cpp
)

target_link_libraries(mocap_tests
    PRIVATE
        Catch2::Catch2WithMain
        core
        detection
        capture
        mocap_warnings
        mocap_sanitizers
)

# register with CTest
include(CTest)
include(Catch)
catch_discover_tests(mocap_tests)