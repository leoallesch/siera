# ──────────────────────────────────────────────────────────────
# SIERA unit tests
# ──────────────────────────────────────────────────────────────

include(FetchContent)

FetchContent_Declare(
    cpputest
    GIT_REPOSITORY https://github.com/cpputest/cpputest.git
    GIT_TAG        master
    GIT_SHALLOW    TRUE
)

# Prevent CppUTest from building its own tests
set(TESTS OFF CACHE BOOL "Disable CppUTest self-tests" FORCE)

FetchContent_MakeAvailable(cpputest)

# ──────────────────────────────────────────────────────────────
# Test sources & mocks
# ──────────────────────────────────────────────────────────────

file(GLOB_RECURSE TEST_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/core/*.cpp"
)

file(GLOB_RECURSE MOCK_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/doubles/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/doubles/*.cpp"
)

add_executable(test_siera
    ${TEST_SOURCES}
    ${MOCK_SOURCES}
    TestRunner.cpp
)

target_include_directories(test_siera
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/doubles/core/timer
        ${CMAKE_CURRENT_SOURCE_DIR}/doubles/core/datastream
        # add more double include paths if needed later
)

target_link_libraries(test_siera
    PRIVATE
        siera
        CppUTest
        CppUTestExt
)

target_compile_options(test_siera PRIVATE -g)

add_test(NAME siera_unit_tests COMMAND test_siera)


# Coverage support
if(SIERA_ENABLE_COVERAGE)
    target_compile_options(test_siera PRIVATE --coverage -O0)
    target_link_options(test_siera    PRIVATE --coverage)
endif()