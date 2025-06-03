#include <action_graph/global_timer.h>
#include <gtest/gtest.h>

using action_graph::GlobalTimer;

#include <atomic>
#include <chrono>

#include <test_clock.h>

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
