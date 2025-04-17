add_library(action_graph)

target_sources(
  action_graph
  PUBLIC
  PRIVATE action.cpp
          include/action_graph/action.h
          include/action_graph/action_sequence.h
          include/action_graph/parallel_actions.h
          include/action_graph/single_action.h)

target_include_directories(action_graph PUBLIC include)

add_library(action_graph::action_graph ALIAS action_graph)
