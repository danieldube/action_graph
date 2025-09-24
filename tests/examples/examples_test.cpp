// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#include <gtest/gtest.h>

#include "example_configurations.h"
#include "example_support.h"

#include <sstream>
#include <string>
#include <vector>

namespace {

std::string CaptureOutputFromGraphExecution() {
  std::ostringstream output;
  {
    examples::ExampleSession session(
        output, "Graph Example", examples::configurations::GraphExecutionYaml());
    auto action = session.BuildAction(session.Configuration());
    action->Execute();
  }
  return output.str();
}

} // namespace

TEST(ExampleConfigurationsTest, OneSecondTriggerHasHeartbeatAction) {
  std::ostringstream output;
  examples::ExampleSession session(output, "Heartbeat",
                                   examples::configurations::
                                       OneSecondTriggerYaml());
  const auto &configuration = session.Configuration();
  ASSERT_TRUE(configuration.IsSequence());
  ASSERT_EQ(configuration.Size(), 1U);
  const auto &trigger = configuration.Get(0).Get("trigger");
  EXPECT_EQ(trigger.Get("name").AsString(), "heartbeat");
  EXPECT_EQ(trigger.Get("period").AsString(), "1 seconds");
  const auto &action = trigger.Get("action");
  EXPECT_EQ(action.Get("name").AsString(), "heartbeat_action");
  EXPECT_EQ(action.Get("type").AsString(), "log_message");
}

TEST(ExampleConfigurationsTest, HighFrequencyScenarioDefinesThreeTriggers) {
  std::ostringstream output;
  examples::ExampleSession session(
      output, "High Frequency",
      examples::configurations::ThreeActionsTenMillisecondsYaml());
  const auto &configuration = session.Configuration();
  ASSERT_TRUE(configuration.IsSequence());
  ASSERT_EQ(configuration.Size(), 3U);
  std::vector<std::string> trigger_names;
  for (std::size_t index = 0; index < configuration.Size(); ++index) {
    const auto &trigger = configuration.Get(index).Get("trigger");
    trigger_names.push_back(trigger.Get("name").AsString());
    EXPECT_EQ(trigger.Get("period").AsString(), "10 milliseconds");
  }
  EXPECT_EQ(trigger_names[0], "alpha");
  EXPECT_EQ(trigger_names[1], "beta");
  EXPECT_EQ(trigger_names[2], "gamma");
}

TEST(ExampleExecutionTest, GraphScenarioLogsAllSteps) {
  const auto log = CaptureOutputFromGraphExecution();
  EXPECT_NE(log.find("Introduce the system to the user."), std::string::npos);
  EXPECT_NE(log.find("Cache is being warmed up."), std::string::npos);
  EXPECT_NE(log.find("Onboarding flow completed."), std::string::npos);
}

TEST(ExampleConfigurationsTest, MonitoredScenarioDefinesTimingMonitor) {
  std::ostringstream output;
  examples::ExampleSession session(output, "Monitored",
                                   examples::configurations::
                                       MonitoredTimerYaml());
  const auto &configuration = session.Configuration();
  ASSERT_TRUE(configuration.IsSequence());
  ASSERT_EQ(configuration.Size(), 1U);
  const auto &trigger = configuration.Get(0).Get("trigger");
  const auto &action = trigger.Get("action");
  ASSERT_TRUE(action.HasKey("decorate"));
  const auto &decorate = action.Get("decorate");
  ASSERT_EQ(decorate.Size(), 1U);
  const auto &monitor = decorate.Get(0);
  EXPECT_EQ(monitor.Get("type").AsString(), "timing_monitor");
  EXPECT_EQ(monitor.Get("duration_limit").AsString(), "30 milliseconds");
  EXPECT_EQ(monitor.Get("expected_period").AsString(), "50 milliseconds");
}

TEST(ExampleContextTest, SummaryListsExecutedActionCount) {
  std::string summary_log;
  {
    std::ostringstream output;
    {
      examples::ExampleSession session(
          output, "Summary Example",
          R"yaml(action:
  name: say_hello
  type: log_message
  message: "Hello from summary."
)yaml");
      auto action = session.BuildAction(session.Configuration());
      action->Execute();
    }
    summary_log = output.str();
  }
  EXPECT_NE(summary_log.find("Summary for Summary Example"),
            std::string::npos);
  EXPECT_NE(summary_log.find("say_hello"), std::string::npos);
  EXPECT_NE(summary_log.find("executed 1 time"), std::string::npos);
}

TEST(ExampleSupportTest, DescribeCountHandlesPluralization) {
  EXPECT_EQ(examples::DescribeCount(1, "apple", "apples"), "1 apple");
  EXPECT_EQ(examples::DescribeCount(2, "apple", "apples"), "2 apples");
}

