// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#ifndef ACTION_GRAPH_TESTS_PARALLEL_ACTIONS_TEST_H_
#define ACTION_GRAPH_TESTS_PARALLEL_ACTIONS_TEST_H_

#include "executor_log.h"
#include <action_graph/parallel_actions.h>
#include <algorithm>
#include <cstddef>
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
  for (std::size_t index = 0; index < 3; ++index) {
    const auto &entry = entries[index];
    EXPECT_EQ(entry.find("start: "), 0);
  }
  for (std::size_t index = 3; index < 6; ++index) {
    const auto &entry = entries[index];
    EXPECT_EQ(entry.find("stop: "), 0);
  }
}

#endif // ACTION_GRAPH_TESTS_PARALLEL_ACTIONS_TEST_H_
