# Copyright (c) 2025 Daniel Dube
#
# This file is part of the action_graph library and is licensed under the MIT
# License. See the LICENSE file in the root directory for full license text.

add_executable(yaml_cpp_configuration_test)
target_sources(yaml_cpp_configuration_test PRIVATE node_test.cpp)

target_link_libraries(
  yaml_cpp_configuration_test
  PRIVATE GTest::gtest_main action_graph::action_graph
          action_graph::yaml_cpp_configuration)

include(CTest)
include(GoogleTest)
gtest_discover_tests(yaml_cpp_configuration_test)
