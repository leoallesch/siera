# SIERA Test Suite

include(FetchContent)

# Fetch CppUTest
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "" FORCE)
FetchContent_Declare(
    cpputest
    GIT_REPOSITORY https://github.com/cpputest/cpputest.git
    GIT_TAG        master
    GIT_SHALLOW    TRUE
)
set(TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(cpputest)

# Collect all test sources
file(GLOB_RECURSE TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/core/*.cpp")
file(GLOB_RECURSE DOUBLE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/doubles/*.c")

add_executable(test_core
    ${TEST_SOURCES}
    ${DOUBLE_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/TestRunner.cpp
)

target_include_directories(test_core
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/doubles/core/timer
)

target_link_libraries(test_core
    PRIVATE
        CppUTest
        CppUTestExt
        siera::core
)

add_test(NAME test_core COMMAND test_core)
