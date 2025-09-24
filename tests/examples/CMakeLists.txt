# Copyright (c) 2025 Daniel Dube
#
# This file is part of the action_graph library and is licensed under the MIT
# License. See the LICENSE file in the root directory for full license text.

add_executable(examples_test)

target_sources(
  examples_test
  PRIVATE examples_test.cpp ${PROJECT_SOURCE_DIR}/src/examples/example_support.cpp)

target_link_libraries(
  examples_test
  PRIVATE GTest::gtest_main action_graph::action_graph
          action_graph::yaml_cpp_configuration)

target_include_directories(
  examples_test PRIVATE ${PROJECT_SOURCE_DIR}/src/examples)

include(GoogleTest)
gtest_discover_tests(examples_test)

