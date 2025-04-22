add_executable(action_graph_test)
target_sources(
  action_graph_test
  PRIVATE action_test.cpp
          action_sequence_test.cpp
          parallel_actions_test.cpp
          single_action_test.cpp
          global_timer_test.cpp
          action_sequence_performance_test.cpp
          parallel_actions_performance_test.cpp)

target_link_libraries(action_graph_test PRIVATE GTest::gtest_main
                                                action_graph::action_graph)

include(CTest)
include(GoogleTest)
gtest_discover_tests(action_graph_test)
