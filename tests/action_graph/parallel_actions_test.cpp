#ifndef ACTION_GRAPH_TESTS_PARALLEL_ACTIONS_TEST_H_
#define ACTION_GRAPH_TESTS_PARALLEL_ACTIONS_TEST_H_

#include "executor_log.h"
#include <action_graph/parallel_actions.h>
#include <gtest/gtest.h>

using action_graph::Action;
using action_graph::ParallelActions;

TEST(ParallelActions, execute) {
  ExecutorLog log;
  auto action1 = std::make_unique<LoggingAction>("action1", log);
  auto action2 = std::make_unique<LoggingAction>("action2", log);
  auto action3 = std::make_unique<LoggingAction>("action3", log);

  action_graph::ParallelActions actions("test_sequence", std::move(action1),
                                        std::move(action2), std::move(action3));
  actions.Execute();

  auto entries = log.GetLog();
  std::for_each_n(std::begin(entries), 3, [](const auto &entry) {
    EXPECT_EQ(entry.find("start: "), 0);
  });
  std::for_each_n(std::begin(entries) + 3, 3, [](const auto &entry) {
    EXPECT_EQ(entry.find("stop: "), 0);
  });
}

#endif // ACTION_GRAPH_TESTS_PARALLEL_ACTIONS_TEST_H_
