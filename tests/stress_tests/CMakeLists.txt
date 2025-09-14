# Copyright (c) 2025 Daniel Dube
#
# This file is part of the action_graph library and is licensed under the MIT
# License. See the LICENSE file in the root directory for full license text.

add_executable(stress_tests)
target_sources(
  stress_tests
  PRIVATE action_sequence_performance_test.cpp
          parallel_actions_performance_test.cpp
          global_timer_timing_monitor_stress_test.cpp)

target_link_libraries(stress_tests PRIVATE GTest::gtest_main
                                           action_graph::action_graph)

target_compile_features(stress_tests PRIVATE cxx_std_14)

include(CTest)
include(GoogleTest)
gtest_discover_tests(stress_tests)
