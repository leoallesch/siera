add_library(simulator-app STATIC app.c counter_view.c)
add_library(simulator::app ALIAS simulator-app)

target_include_directories(simulator-app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(simulator-app PUBLIC siera)

if(TARGET lvgl)
  target_link_libraries(simulator-app PUBLIC lvgl)
endif()
target_compile_options(simulator-app PRIVATE -Wall -Wextra)
set_target_properties(simulator-app PROPERTIES C_STANDARD 11)
