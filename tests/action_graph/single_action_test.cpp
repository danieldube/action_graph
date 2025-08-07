// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#include <gtest/gtest.h>

#include <action_graph/single_action.h>

using action_graph::SingleAction;

TEST(SingleAction, function) {
  bool was_executed = false;
  SingleAction action("test_action",
                      [&was_executed]() { was_executed = true; });
  action.Execute();
  EXPECT_TRUE(was_executed);
}
