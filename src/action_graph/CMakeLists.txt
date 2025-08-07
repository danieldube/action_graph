# Copyright (c) 2025 Daniel Dube
#
# This file is part of the action_graph library and is licensed under the MIT
# License. See the LICENSE file in the root directory for full license text.

add_library(action_graph)

target_sources(
  action_graph
  PUBLIC
  PRIVATE action.cpp
          builder/builder.cpp
          include/action_graph/action.h
          include/action_graph/action_sequence.h
          include/action_graph/builder/builder.h
          include/action_graph/parallel_actions.h
          include/action_graph/single_action.h
          builder/parse_duration.cpp
          include/action_graph/builder/parse_duration.h
          include/action_graph/builder/generic_action_builder.h
          include/action_graph/builder/generic_action_decorator.h
          builder/generic_action_decorator.cpp
          include/action_graph/builder/configuration_node.h
          builder/generic_action_builder.cpp
          include/action_graph/global_timer/global_timer.h
          include/action_graph/global_timer/trigger.h
          global_timer/trigger.cpp
          include/action_graph/execution_observer.h
          include/action_graph/observable_action.h
          include/action_graph/decorated_action.h)

target_include_directories(action_graph PUBLIC include)

add_library(action_graph::action_graph ALIAS action_graph)
