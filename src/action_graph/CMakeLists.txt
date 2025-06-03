find_package(yaml-cpp REQUIRED)

add_library(action_graph builder/generic_action_builder.cpp
                         include/action_graph/builder/generic_action_builder.h)

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
          builder/generic_action_builder.cpp)

target_include_directories(action_graph PUBLIC include)

add_library(action_graph::action_graph ALIAS action_graph)

target_link_libraries(action_graph PUBLIC yaml-cpp::yaml-cpp)
