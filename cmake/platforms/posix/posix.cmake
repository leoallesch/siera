list(APPEND SIERA_PLATFORMS "posix")

if(SIERA_PLATFORM STREQUAL "posix")
    # Host (POSIX) platform — no toolchain override required.
    # Future pre-project() setup (sanitizers, forced stdlib) goes here.
    message(STATUS "SIERA: building for posix (host)")
endif()
