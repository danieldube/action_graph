// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#include <action_graph/global_timer/trigger.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <atomic>
#include <chrono>

using std::chrono::milliseconds;
using std::chrono::seconds;

class ThreadSafeLog {
public:
  void Log(const std::string &message) {
    std::lock_guard<std::mutex> lock(mutex_);
    log_.push_back(message);
  }

  std::vector<std::string> GetLog() {
    std::lock_guard<std::mutex> lock(mutex_);
    return log_;
  }

private:
  std::vector<std::string> log_;
  std::mutex mutex_;
};

class TriggerTest : public ::testing::Test {
protected:
  using Trigger = action_graph::Trigger;

  TriggerTest() : trigger([this]() { TriggerFunction(); }) {}

  void TriggerFunction() {
    log.Log("running");
    should_finish = false;
    while (!should_finish.load()) {
      std::this_thread::yield();
    }
    log.Log("finished");
  }

  static void GiveTriggerThreadTimeToProcess() {
    constexpr std::chrono::milliseconds kOneMillisecond{1};
    std::this_thread::sleep_for(kOneMillisecond);
  }

  ThreadSafeLog log;
  std::atomic<bool> should_finish{false};

  Trigger trigger;
};

TEST_F(TriggerTest, trigger_once) {
  std::vector<std::string> expected_log{};
  EXPECT_EQ(log.GetLog(), expected_log);

  trigger.TriggerAsynchronously();
  GiveTriggerThreadTimeToProcess();
  should_finish = true;
  trigger.WaitUntilTriggerIsFinished();
  expected_log = {"running", "finished"};
  EXPECT_EQ(log.GetLog(), expected_log);
}

TEST_F(TriggerTest, trigger_a_second_time_while_running) {
  std::vector<std::string> expected_log{};

  trigger.TriggerAsynchronously();
  trigger.TriggerAsynchronously();
  GiveTriggerThreadTimeToProcess();

  expected_log = {"running"};
  EXPECT_EQ(log.GetLog(), std::vector<std::string>{"running"});

  should_finish = true;
  trigger.WaitUntilTriggerIsFinished();

  expected_log = {"running", "finished"};
  EXPECT_EQ(log.GetLog(), expected_log);
}

TEST_F(TriggerTest, trigger_twice) {
  std::vector<std::string> expected_log{};

  trigger.TriggerAsynchronously();
  GiveTriggerThreadTimeToProcess();
  should_finish = true;
  trigger.WaitUntilTriggerIsFinished();

  trigger.TriggerAsynchronously();
  GiveTriggerThreadTimeToProcess();
  should_finish = true;
  trigger.WaitUntilTriggerIsFinished();

  expected_log = {"running", "finished", "running", "finished"};
  EXPECT_EQ(log.GetLog(), expected_log);
}
