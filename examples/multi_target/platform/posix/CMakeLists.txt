add_executable(multi_target main.c)
target_link_libraries(multi_target PRIVATE multi_target::app siera::posix)
target_compile_options(multi_target PRIVATE -Wall -Wextra)
set_target_properties(multi_target PROPERTIES C_STANDARD 11)
