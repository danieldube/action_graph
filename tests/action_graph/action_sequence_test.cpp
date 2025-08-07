// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#include "executor_log.h"
#include <action_graph/action_sequence.h>
#include <gtest/gtest.h>

using action_graph::Action;
using action_graph::ActionSequence;

TEST(ActionSequence, create_from_variadic_parameters) {
  ExecutorLog log;
  auto action1 = std::make_unique<LoggingAction>("action1", std::ref(log));
  auto action2 = std::make_unique<LoggingAction>("action2", std::ref(log));

  ActionSequence sequence("test_sequence", std::move(action1),
                          std::move(action2));

  sequence.Execute();

  std::vector<std::string> expected_log = {"start: action1", "stop: action1",
                                           "start: action2", "stop: action2"};

  EXPECT_EQ(log.GetLog(), expected_log);
}

TEST(ActionSequence, create_from_vector) {
  ExecutorLog log;
  std::vector<std::unique_ptr<Action>> actions;
  actions.push_back(std::make_unique<LoggingAction>("action1", std::ref(log)));
  actions.push_back(std::make_unique<LoggingAction>("action2", std::ref(log)));

  ActionSequence sequence("test_sequence", std::move(actions));

  sequence.Execute();

  std::vector<std::string> expected_log = {"start: action1", "stop: action1",
                                           "start: action2", "stop: action2"};

  EXPECT_EQ(log.GetLog(), expected_log);
}

TEST(ActionSequence, execute) {
  ExecutorLog log;
  auto action1 = std::make_unique<LoggingAction>("action1", std::ref(log));
  auto action2 = std::make_unique<LoggingAction>("action2", std::ref(log));
  auto action3 = std::make_unique<LoggingAction>("action3", std::ref(log));

  action_graph::ActionSequence sequence("test_sequence", std::move(action1),
                                        std::move(action2), std::move(action3));
  sequence.Execute();

  std::vector<std::string> expected_log = {"start: action1", "stop: action1",
                                           "start: action2", "stop: action2",
                                           "start: action3", "stop: action3"};

  EXPECT_EQ(log.GetLog(), expected_log);
}
