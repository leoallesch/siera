add_executable(simulator main.c)
target_link_libraries(simulator PRIVATE simulator::app siera)
target_include_directories(simulator PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_options(simulator PRIVATE -Wall -Wextra -g)
set_target_properties(simulator PROPERTIES C_STANDARD 11)