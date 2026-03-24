# siera_add_module(
#   SRCS     [src1.c ...]    # optional — omit for header-only modules
#   INCLUDES [dir1 ...]      # relative to caller dir; defaults to "include"
#   LINKS    [lib ...]       # extra link dependencies (e.g. idf::driver)
# )
#
# Appends sources/includes/links to the siera INTERFACE target.

macro(siera_add_module)
    cmake_parse_arguments(_MOD "" "" "SRCS;INCLUDES;LINKS" ${ARGN})

    if(NOT _MOD_INCLUDES)
        set(_MOD_INCLUDES include)
    endif()

    set(_abs_srcs "")
    foreach(_src IN LISTS _MOD_SRCS)
        list(APPEND _abs_srcs ${CMAKE_CURRENT_SOURCE_DIR}/${_src})
    endforeach()

    set(_abs_includes "")
    foreach(_dir IN LISTS _MOD_INCLUDES)
        list(APPEND _abs_includes ${CMAKE_CURRENT_SOURCE_DIR}/${_dir})
    endforeach()

    if(_abs_srcs)
        target_sources(siera INTERFACE ${_abs_srcs})
    endif()

    foreach(_idir IN LISTS _abs_includes)
        target_include_directories(siera INTERFACE
            $<BUILD_INTERFACE:${_idir}>
            $<INSTALL_INTERFACE:include>)
    endforeach()

    if(_MOD_LINKS)
        target_link_libraries(siera INTERFACE ${_MOD_LINKS})
    endif()
endmacro()
