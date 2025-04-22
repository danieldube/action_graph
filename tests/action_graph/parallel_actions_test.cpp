#ifndef ACTION_GRAPH_TESTS_PARALLEL_ACTIONS_TEST_H_
#define ACTION_GRAPH_TESTS_PARALLEL_ACTIONS_TEST_H_

#include "executor_log.h"
#include <action_graph/parallel_actions.h>
#include <algorithm>
#include <gtest/gtest.h>

using action_graph::Action;
using action_graph::ParallelActions;

TEST(ParallelActions, create_from_variadic_parameters) {
  ExecutorLog log;
  auto action1 = std::make_unique<LoggingAction>("action1", log);
  auto action2 = std::make_unique<LoggingAction>("action2", log);

  action_graph::ParallelActions branches("test_sequence", std::move(action1),
                                         std::move(action2));
  branches.Execute();

  auto entries = log.GetLog();
  EXPECT_EQ(entries.size(), 4);
}

TEST(ParallelActions, create_from_vector) {
  ExecutorLog log;

  std::vector<std::unique_ptr<Action>> actions;

  actions.push_back(std::make_unique<LoggingAction>("action1", log));
  actions.push_back(std::make_unique<LoggingAction>("action2", log));

  action_graph::ParallelActions branches("test_sequence", std::move(actions));
  branches.Execute();

  auto entries = log.GetLog();
  EXPECT_EQ(entries.size(), 4);
}

TEST(ParallelActions, execute) {
  ExecutorLog log;
  auto action1 = std::make_unique<LoggingAction>("action1", log);
  auto action2 = std::make_unique<LoggingAction>("action2", log);
  auto action3 = std::make_unique<LoggingAction>("action3", log);

  action_graph::ParallelActions branches("test_sequence", std::move(action1),
                                         std::move(action2),
                                         std::move(action3));
  branches.Execute();

  auto entries = log.GetLog();
  std::for_each_n(std::begin(entries), 3, [](const auto &entry) {
    EXPECT_EQ(entry.find("start: "), 0);
  });
  std::for_each_n(std::begin(entries) + 3, 3, [](const auto &entry) {
    EXPECT_EQ(entry.find("stop: "), 0);
  });
}

#endif // ACTION_GRAPH_TESTS_PARALLEL_ACTIONS_TEST_H_
