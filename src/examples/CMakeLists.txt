# Copyright (c) 2025 Daniel Dube
#
# This file is part of the action_graph library and is licensed under the MIT
# License. See the LICENSE file in the root directory for full license text.

add_executable(
  action_graph_examples
  composite_graph_example.cpp
  console_log.cpp
  examples_main.cpp
  high_frequency_example.cpp
  logging_builder.cpp
  monitored_graph_example.cpp
  single_second_example.cpp)

target_link_libraries(
  action_graph_examples
  PRIVATE action_graph::action_graph action_graph::yaml_cpp_configuration
          action_graph::file_log)

target_compile_features(action_graph_examples PRIVATE cxx_std_14)
set_target_properties(action_graph_examples PROPERTIES CXX_EXTENSIONS OFF)
