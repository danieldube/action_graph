# Copyright (c) 2025 Daniel Dube
#
# This file is part of the action_graph library and is licensed under the MIT
# License. See the LICENSE file in the root directory for full license text.

add_executable(
  action_graph_examples
  example_support.cpp graph_execution_example.cpp library_usage_examples.cpp
  monitored_timer_example.cpp one_second_trigger_example.cpp
  three_actions_ten_milliseconds_example.cpp)

target_link_libraries(
  action_graph_examples PRIVATE action_graph::action_graph
                                action_graph::yaml_cpp_configuration)
