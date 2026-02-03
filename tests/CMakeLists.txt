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

# Add module tests
add_subdirectory(core)

if(SIERA_ENABLE_UI)
    # add_subdirectory(ui)  # when UI tests exist
endif()
