# Copyright (c) 2025 Daniel Dube
#
# This file is part of the action_graph library and is licensed under the MIT
# License. See the LICENSE file in the root directory for full license text.

add_executable(file_log_test)
target_sources(file_log_test PRIVATE file_log_test.cpp)

target_link_libraries(file_log_test PRIVATE GTest::gtest_main
                                            action_graph::file_log)

target_compile_features(file_log_test PRIVATE cxx_std_14)
set_target_properties(file_log_test PROPERTIES CXX_EXTENSIONS OFF)

include(CTest)
include(GoogleTest)
gtest_discover_tests(file_log_test)
