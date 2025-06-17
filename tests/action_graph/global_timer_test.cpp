#include <action_graph/global_timer.h>
#include <gtest/gtest.h>

using action_graph::GlobalTimer;

#include <atomic>
#include <chrono>

#include <test_clock.h>

using std::chrono::milliseconds;
using std::chrono::seconds;

class GlobalTimerTest : public ::testing::Test {
protected:
  void SetUp() override { TestClock::reset(); }

  void TearDown() override { TestClock::reset(); }
};

TEST_F(GlobalTimerTest, set_trigger) {
  auto timer = GlobalTimer<TestClock>{};
  timer.SetTriggerTime(std::chrono::milliseconds{2}, []() {});
}

TEST_F(GlobalTimerTest, one_trigger) {
  size_t trigger_counter{0};
  {
    auto timer = GlobalTimer<TestClock>{};

    timer.SetTriggerTime(std::chrono::milliseconds{2},
                         [&trigger_counter]() { ++trigger_counter; });

    constexpr std::chrono::milliseconds kOneMillisecond{1};
    for (int loop = 1; loop < 6; ++loop) {
      TestClock::advance_time(kOneMillisecond);
      timer.WaitOneCycle();
    }
  }
  EXPECT_EQ(trigger_counter, 2);
}

TEST_F(GlobalTimerTest, two_triggers) {
  size_t trigger_counter_one{0};
  size_t trigger_counter_two{0};
  {
    auto timer = GlobalTimer<TestClock>{};

    timer.SetTriggerTime(std::chrono::milliseconds{1},
                         [&trigger_counter_one]() { ++trigger_counter_one; });
    timer.SetTriggerTime(std::chrono::milliseconds{2},
                         [&trigger_counter_two]() { ++trigger_counter_two; });

    constexpr std::chrono::milliseconds kOneMillisecond{1};
    for (int loop = 1; loop < 6; ++loop) {
      TestClock::advance_time(kOneMillisecond);
      timer.WaitOneCycle();
    }
  }
  EXPECT_EQ(trigger_counter_one, 5);
  EXPECT_EQ(trigger_counter_two, 2);
}

TEST_F(GlobalTimerTest, time_jump_backwards) {
  size_t trigger_counter_one{0};
  auto timer = GlobalTimer<TestClock>{};

  timer.SetTriggerTime(std::chrono::milliseconds{10},
                       [&trigger_counter_one]() { ++trigger_counter_one; });

  TestClock::advance_time(std::chrono::milliseconds{15});
  timer.WaitOneCycle();
  EXPECT_EQ(trigger_counter_one, 1);

  TestClock::reset();
  timer.WaitOneCycle();
  EXPECT_EQ(trigger_counter_one, 1);

  TestClock::advance_time(std::chrono::milliseconds{10});
  timer.WaitOneCycle();
  EXPECT_EQ(trigger_counter_one, 2);
}
