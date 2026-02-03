# Core Module Tests

add_executable(test_core
    ${CMAKE_CURRENT_SOURCE_DIR}/data_structures/test_list.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/data_structures/test_queue.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/event/test_event.cpp
    ${CMAKE_SOURCE_DIR}/tests/TestRunner.cpp
)

target_link_libraries(test_core
    PRIVATE
        CppUTest
        CppUTestExt
        siera::core
)

add_test(NAME test_core COMMAND test_core)
