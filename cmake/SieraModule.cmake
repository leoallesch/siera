# siera_add_module(
#   NAME     <module_name>
#   SRCS     [src1.c ...]    # optional — omit for header-only modules
#   INCLUDES [dir1 ...]      # relative to caller dir; defaults to "include"
#   LINKS    [lib ...]       # host only — extra link dependencies
# )
#
# On host:     appends sources/includes to the siera INTERFACE target.
# On ESP-IDF:  appends to SIERA_SRCS / SIERA_INCLUDE_DIRS for the single
#              idf_component_register in the root CMakeLists.txt.
#              IDF component dependencies are declared via idf_requires.cmake
#              in each driver directory, not through this macro.

macro(siera_add_module)
    cmake_parse_arguments(_MOD "" "NAME" "SRCS;INCLUDES;LINKS" ${ARGN})

    if(NOT _MOD_NAME)
        message(FATAL_ERROR "siera_add_module: NAME is required")
    endif()

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

    if(ESP_PLATFORM)
        list(APPEND SIERA_SRCS         ${_abs_srcs})
        list(APPEND SIERA_INCLUDE_DIRS ${_abs_includes})
        set(SIERA_SRCS         ${SIERA_SRCS}         PARENT_SCOPE)
        set(SIERA_INCLUDE_DIRS ${SIERA_INCLUDE_DIRS} PARENT_SCOPE)
    else()
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
    endif()
endmacro()
