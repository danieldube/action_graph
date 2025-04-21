#include <action_graph/global_timer.h>
#include <gtest/gtest.h>

using action_graph::GlobalTimer;

#include <atomic>
#include <chrono>

// NOLINTBEGIN(*identifier-naming)
class TestClock {
public:
  using rep = int64_t;
  using period = std::milli;
  using duration = std::chrono::duration<rep, period>;
  using time_point = std::chrono::time_point<TestClock, duration>;

  static constexpr bool is_steady = true;

  static time_point now() noexcept { return time_point(current_time_.load()); }

  static void advance_time(const duration &delta) noexcept {
    current_time_ = current_time_.load() + delta;
  }

  static void reset() noexcept { current_time_ = duration{0}; }

private:
  static std::atomic<duration> current_time_;
};
// NOLINTEND(*identifier-naming)

std::atomic<TestClock::duration> TestClock::current_time_ =
    TestClock::duration{0};

using std::chrono::milliseconds;
using std::chrono::seconds;

TEST(TestClock, init) {
  TestClock::reset();
  EXPECT_EQ(TestClock::now().time_since_epoch(), std::chrono::seconds{0});
}

TEST(TestClock, advance) {
  TestClock::reset();
  TestClock::advance_time(seconds{5});
  EXPECT_EQ(TestClock::now().time_since_epoch(), seconds{5});
  TestClock::advance_time(milliseconds{1});
  EXPECT_EQ(TestClock::now().time_since_epoch(), milliseconds{5001});
}

TEST(TestClock, reset) {
  TestClock::reset();
  TestClock::advance_time(seconds{5});
  TestClock::reset();
  EXPECT_EQ(TestClock::now().time_since_epoch(), seconds{0});
}

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
