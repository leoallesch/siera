# Include before project() in a consumer CMakeLists.txt:
#
#   set(SIERA_ROOT path/to/siera)
#   include(${SIERA_ROOT}/cmake/SieraPlatform.cmake)
#   project(my_app ...)
#
# Pass -DSIERA_PLATFORM=<name> at configure time.

if(DEFINED SIERA_PLATFORM)
    string(TOLOWER "${SIERA_PLATFORM}" _platform)
    file(GLOB _siera_platform_cmake "${CMAKE_CURRENT_LIST_DIR}/platforms/${_platform}/*.cmake")
    foreach(_f IN LISTS _siera_platform_cmake)
        include("${_f}")
    endforeach()
endif()
