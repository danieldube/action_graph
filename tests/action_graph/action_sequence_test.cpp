#include "executor_log.h"
#include <action_graph/action_sequence.h>
#include <gtest/gtest.h>

using action_graph::Action;
using action_graph::ActionSequence;

TEST(ActionSequence, test) {
  ExecutorLog log;
  auto action1 = std::make_unique<LoggingAction>("action1", std::ref(log));
  auto action2 = std::make_unique<LoggingAction>("action2", std::ref(log));
  auto action3 = std::make_unique<LoggingAction>("action3", std::ref(log));

  action_graph::ActionSequence sequence("test_sequence", std::move(action1),
                                        std::move(action2), std::move(action3));
  sequence.Execute();

  EXPECT_EQ(log.GetLog().size(), 3);
}
