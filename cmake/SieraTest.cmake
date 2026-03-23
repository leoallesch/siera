# siera_add_test(
#   NAME          <target_name>
#   SRCS          <src1.cpp> [src2.cpp ...]
#   EXTRA_LINKS   [lib1 ...]              # optional — additional libraries beyond siera + CppUTest
#   INCLUDE_DIRS  [dir1 ...]              # optional — extra include directories
# )
#
# Creates a CppUTest executable and registers it with ctest.
# siera and TestMain.cpp are linked automatically.

function(siera_add_test)
    cmake_parse_arguments(ARG "" "NAME" "SRCS;EXTRA_LINKS;INCLUDE_DIRS" ${ARGN})

    if(NOT ARG_NAME)
        message(FATAL_ERROR "siera_add_test: NAME is required")
    endif()

    if(NOT ARG_SRCS)
        message(FATAL_ERROR "siera_add_test: SRCS is required")
    endif()

    # ── Executable ────────────────────────────────────────────────────────────

    add_executable(${ARG_NAME}
        ${ARG_SRCS}
        ${CMAKE_SOURCE_DIR}/tests/TestMain.cpp)

    # ── Link libraries ────────────────────────────────────────────────────────

    target_link_libraries(${ARG_NAME} PRIVATE siera CppUTest CppUTestExt)

    if(ARG_EXTRA_LINKS)
        target_link_libraries(${ARG_NAME} PRIVATE ${ARG_EXTRA_LINKS})
    endif()

    # ── Include directories ───────────────────────────────────────────────────

    if(ARG_INCLUDE_DIRS)
        target_include_directories(${ARG_NAME} PRIVATE ${ARG_INCLUDE_DIRS})
    endif()

    # ── Properties ───────────────────────────────────────────────────────────

    set_target_properties(${ARG_NAME} PROPERTIES CXX_STANDARD 20)
    target_compile_options(${ARG_NAME} PRIVATE -Wall -Wextra)

    # ── Register with ctest ───────────────────────────────────────────────────

    add_test(NAME ${ARG_NAME} COMMAND ${ARG_NAME})

endfunction()
