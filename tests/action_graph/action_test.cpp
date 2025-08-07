// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#include <action_graph/action.h>
#include <gtest/gtest.h>

using action_graph::Action;

class TestAction final : public Action {
public:
  TestAction(std::string name, bool &mark_when_execute, bool &mark_when_deleted)
      : Action(std::move(name)), mark_when_execute_{mark_when_execute},
        mark_when_deleted_{mark_when_deleted} {}

  ~TestAction() override { mark_when_deleted_ = true; }

  void Execute() override { mark_when_execute_ = true; }

private:
  bool &mark_when_execute_;
  bool &mark_when_deleted_;
};

TEST(Action, execute) {
  bool was_executed = false;
  bool was_deleted = false;
  TestAction action("test_action", was_executed, was_deleted);
  action.Execute();
  EXPECT_TRUE(was_executed);
}

TEST(Action, delete_via_base_class) {
  bool was_executed = false;
  bool was_deleted = false;
  {
    std::unique_ptr<Action> action =
        std::make_unique<TestAction>("test_action", was_executed, was_deleted);
  }
  EXPECT_TRUE(was_deleted);
}
