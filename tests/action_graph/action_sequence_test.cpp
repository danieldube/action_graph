#include "executor_log.h"
#include <action_graph/action_sequence.h>
#include <gtest/gtest.h>

using action_graph::Action;
using action_graph::ActionSequence;

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
