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

TEST_F(GlobalTimerTest, mix_high_frequency_with_low_frequency) {
  ThreadSafeLog log;
  {
    auto timer = GlobalTimer<TestClock>{};

    timer.SetTriggerTime(std::chrono::milliseconds{3}, [&log]() {
      auto now = TestClock::now();
      log.Log("execute Alice at " +
              std::to_string(now.time_since_epoch().count()));
      auto wait_until = TestClock::time_point(std::chrono::milliseconds{5});
      while (TestClock::now() < wait_until) {
      };
    });
    timer.SetTriggerTime(std::chrono::milliseconds{1}, [&log]() {
      auto now = TestClock::now();
      log.Log("execute Bob at " +
              std::to_string(now.time_since_epoch().count()));
    });

    constexpr std::chrono::milliseconds kOneMillisecond{1};
    for (int loop = 1; loop < 6; ++loop) {
      TestClock::advance_time(kOneMillisecond);
      std::this_thread::sleep_for(std::chrono::milliseconds{10});
    }
  }

  std::vector<std::string> expected_log = {
      "execute Bob at 1", "execute Bob at 2", "execute Alice at 3",
      "execute Bob at 3", "execute Bob at 4", "execute Bob at 5"};

  const auto log_messages = log.GetLog();
  for (const auto expected_log_entry : expected_log) {
    auto log_entry =
        std::find(log_messages.begin(), log_messages.end(), expected_log_entry);
    EXPECT_NE(log_entry, log_messages.end())
        << "Expected log entry not found: " << expected_log_entry;
  }
}

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
