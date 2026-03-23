# Shared mock/stub sources used across test suites.
# Add an INTERFACE library here so individual test targets can link against it:
#
#   add_library(siera_mocks INTERFACE)
#   target_sources(siera_mocks INTERFACE mock_foo.c mock_bar.c)
#   target_include_directories(siera_mocks INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
#
# Then in a test's CMakeLists.txt:
#   siera_add_test(NAME test_xyz SRCS test_xyz.cpp EXTRA_LINKS siera_mocks)
