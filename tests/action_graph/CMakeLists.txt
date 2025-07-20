add_executable(action_graph_test)
target_sources(
  action_graph_test
  PRIVATE action_test.cpp
          action_sequence_test.cpp
          builder/builder_test.cpp
          parallel_actions_test.cpp
          single_action_test.cpp
          test_clock.h
          test_clock.cpp
          test_clock_test.cpp
          global_timer/global_timer_test.cpp
          global_timer/trigger_test.cpp
          action_sequence_performance_test.cpp
          parallel_actions_performance_test.cpp
          observable_action_test.cpp
          builder/parse_duration_test.cpp
          builder/generic_action_builder_test.cpp
          builder/callback_action.cpp
          builder/callback_action.h
          builder/configuration_node_test.cpp
          decorated_action_test.cpp)

target_link_libraries(
  action_graph_test PRIVATE GTest::gtest_main action_graph::action_graph
                            action_graph::native_configuration)

target_include_directories(action_graph_test PRIVATE ${CMAKE_CURRENT_LIST_DIR})

include(CTest)
include(GoogleTest)
gtest_discover_tests(action_graph_test)
