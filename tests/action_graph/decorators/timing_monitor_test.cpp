// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#include <action_graph/decorators/timing_monitor.h>
#include <future>
#include <gtest/gtest.h>
#include <test_clock.h>
#include <thread>

using action_graph::Action;

template <typename Clock> class WaitingAction final : public Action {
public:
  explicit WaitingAction() : Action("WaitingAction") {}

  void Execute() override {
    const auto start = Clock::now();
    while (Clock::now() - start == Clock::duration::zero()) {
      // busy wait
    }
  }

private:
  typename Clock::duration wait_duration_;
};

using action_graph::decorators::TimingMonitor;
using namespace std::chrono_literals;

class TimingMonitorTest : public ::testing::Test {
protected:
  void SetUp() override {
    TestClock::reset();
    auto action = std::make_unique<WaitingAction<TestClock>>();
    timing_action = std::make_unique<TimingMonitor<TestClock>>(
        std::move(action), action_duration,
        [this]() { exceeded_duration = true; }, period,
        [this]() { trigger_miss = true; });
  }

  void ExecuteAction(const TestClock::duration &execute_duration) const {
    auto future =
        std::async(std::launch::async, [this]() { timing_action->Execute(); });
    std::this_thread::sleep_for(setup_duration);
    TestClock::advance_time(execute_duration);
    future.get();
  }

  TestClock::duration action_duration{100ms};
  TestClock::duration period{200ms};
  TestClock::duration duration_between_calls{period - action_duration};
  TestClock::duration setup_duration{30ms};

  std::unique_ptr<TimingMonitor<TestClock>> timing_action;

  bool exceeded_duration = false;
  bool trigger_miss = false;
};

TEST_F(TimingMonitorTest, in_duration) {
  ExecuteAction(action_duration - 10ms);
  EXPECT_FALSE(exceeded_duration);
}

TEST_F(TimingMonitorTest, at_duration) {
  ExecuteAction(action_duration);
  EXPECT_FALSE(exceeded_duration);
}

TEST_F(TimingMonitorTest, exceed_duration) {
  ExecuteAction(action_duration + 10ms);
  EXPECT_TRUE(exceeded_duration);
}

TEST_F(TimingMonitorTest, in_trigger_period) {
  ExecuteAction(action_duration);
  TestClock::advance_time(duration_between_calls);
  ExecuteAction(action_duration);
  EXPECT_FALSE(trigger_miss);
}

TEST_F(TimingMonitorTest, first_in_then_exceeds_trigger_period) {
  ExecuteAction(action_duration);
  TestClock::advance_time(duration_between_calls);
  ExecuteAction(action_duration);
  EXPECT_FALSE(trigger_miss);
  TestClock::advance_time(duration_between_calls * 2);
  ExecuteAction(action_duration);
  EXPECT_TRUE(trigger_miss);
}

TEST_F(TimingMonitorTest, exceeds_trigger_period) {
  ExecuteAction(action_duration);
  TestClock::advance_time(duration_between_calls + 1ms);
  ExecuteAction(action_duration);
  EXPECT_TRUE(trigger_miss);
}
