# project.cmake creates ${CMAKE_PROJECT_NAME}.elf automatically.
# We add our sources and link libraries to it.
set(_elf ${CMAKE_PROJECT_NAME}.elf)

target_sources(${_elf} PRIVATE main.c)
target_link_libraries(${_elf} PRIVATE
    multi_target::app
    siera
)
