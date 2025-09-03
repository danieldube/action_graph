# Copyright (c) 2025 Daniel Dube
#
# This file is part of the action_graph library and is licensed under the MIT
# License. See the LICENSE file in the root directory for full license text.

add_library(action_graph)

target_sources(
  action_graph
  PRIVATE action.cpp builder/builder.cpp builder/parse_duration.cpp
          builder/generic_action_decorator.cpp
          builder/generic_action_builder.cpp global_timer/trigger.cpp
  PUBLIC FILE_SET
         action_graph_headers
         TYPE
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_SOURCE_DIR}/include
         FILES
         include/action_graph/action.h
         include/action_graph/action_sequence.h
         include/action_graph/builder/builder.h
         include/action_graph/parallel_actions.h
         include/action_graph/log.h
         include/action_graph/single_action.h
         include/action_graph/builder/parse_duration.h
         include/action_graph/builder/generic_action_builder.h
         include/action_graph/builder/generic_action_decorator.h
         include/action_graph/builder/configuration_node.h
         include/action_graph/global_timer/global_timer.h
         include/action_graph/global_timer/trigger.h
         include/action_graph/decorators/execution_observer.h
         include/action_graph/decorators/observable_action.h
         include/action_graph/decorators/decorated_action.h
         include/action_graph/decorators/timing_monitor.h)

target_include_directories(action_graph PUBLIC include)

add_library(action_graph::action_graph ALIAS action_graph)
