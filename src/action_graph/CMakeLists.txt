add_library(action_graph)

target_sources(
  action_graph
  PUBLIC
  PRIVATE action.cpp)

target_include_directories(action_graph PUBLIC include)

add_library(action_graph::action_graph ALIAS action_graph)
