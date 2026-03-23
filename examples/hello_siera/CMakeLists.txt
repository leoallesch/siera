add_executable(hello_siera main.c)
target_include_directories(hello_siera PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(hello_siera PRIVATE siera)
