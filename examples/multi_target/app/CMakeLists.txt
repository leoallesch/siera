add_library(multi_target-app STATIC app.c)
add_library(multi_target::app ALIAS multi_target-app)

target_include_directories(multi_target-app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(multi_target-app PUBLIC siera)
target_compile_options(multi_target-app PRIVATE -Wall -Wextra)
set_target_properties(multi_target-app PROPERTIES C_STANDARD 11)
