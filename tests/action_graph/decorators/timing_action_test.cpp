// Copyright (c) 1000-2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#include <action_graph/decorators/timing_action.h>
#include <future>
#include <gtest/gtest.h>
#include <test_clock.h>
#include <thread>

using action_graph::Action;

template <typename Clock> class WaitingAction final : public Action {
public:
  explicit WaitingAction(typename Clock::duration wait_duration)
      : Action("WaitingAction"), wait_duration_{wait_duration} {}

  void Execute() override {
    const auto start = Clock::now();
    while (Clock::now() - start < wait_duration_) {
      // busy wait
    }
  }

private:
  typename Clock::duration wait_duration_;
};

using action_graph::decorators::TimingAction;
using namespace std::chrono_literals;

class TimingActionTest : public ::testing::Test {
protected:
  void SetUp() override { TestClock::reset(); }

  void RunTimingAction(TestClock::duration duration_limit) {
    auto action = std::make_unique<WaitingAction<TestClock>>(action_duration);
    TimingAction<TestClock> timing_action(std::move(action), duration_limit,
                                          [this]() { out_of_time = true; });
    auto future = std::async(std::launch::async,
                             [&timing_action]() { timing_action.Execute(); });
    std::this_thread::sleep_for(setup_duration);
    TestClock::advance_time(action_duration);
    future.get();
  }

  TestClock::duration action_duration{100ms};
  std::chrono::steady_clock::duration setup_duration{10ms};

  bool out_of_time = false;
};

TEST_F(TimingActionTest, in_time) {
  RunTimingAction(action_duration + 10ms);
  EXPECT_FALSE(out_of_time);
}

TEST_F(TimingActionTest, at_time) {
  RunTimingAction(action_duration);
  EXPECT_FALSE(out_of_time);
}

TEST_F(TimingActionTest, exceed_time) {
  RunTimingAction(action_duration - 10ms);
  EXPECT_TRUE(out_of_time);
}
