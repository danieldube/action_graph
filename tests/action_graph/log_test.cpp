// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#include <gtest/gtest.h>

#include <action_graph/log.h>
#include <test_log.h>

using action_graph::Log;

TEST(Log, log_message) {
  TestLog test_log;
  Log &log = test_log;

  log.LogMessage("Test");

  const std::vector<std::string> expected_messages{"Message: Test"};
  EXPECT_EQ(test_log.log, expected_messages);
}

TEST(Log, log_multiple_messages) {
  TestLog test_log;
  Log &log = test_log;

  log.LogMessage("Test 1");
  log.LogMessage("Test 2");

  const std::vector<std::string> expected_messages = {"Message: Test 1",
                                                      "Message: Test 2"};
  EXPECT_EQ(test_log.log, expected_messages);
}

TEST(Log, log_error) {
  TestLog test_log;
  Log &log = test_log;

  log.LogError("Test");

  const std::vector<std::string> expected_messages = {"Error: Test"};
  EXPECT_EQ(test_log.log, expected_messages);
}
