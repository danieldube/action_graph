# Copyright (c) 2025 Daniel Dube
#
# This file is part of the action_graph library and is licensed under the MIT
# License. See the LICENSE file in the root directory for full license text.

add_executable(native_configuration_test)
target_sources(
  native_configuration_test PRIVATE map_node_test.cpp scalar_node_test.cpp
                                    sequence_node_test.cpp)

target_link_libraries(
  native_configuration_test PRIVATE GTest::gtest_main
                                    action_graph::native_configuration)

include(CTest)
include(GoogleTest)
gtest_discover_tests(native_configuration_test)
