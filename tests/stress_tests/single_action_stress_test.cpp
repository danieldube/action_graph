// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#include <action_graph/action.h>
#include <action_graph/decorators/timing_monitor.h>
#include <action_graph/global_timer/global_timer.h>
#include <atomic>
#include <chrono>
#include <cmath>
#include <gtest/gtest.h>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

using namespace std::chrono;
using action_graph::Action;
using action_graph::GlobalTimer;
using action_graph::decorators::TimingMonitor;

void BurnCpuCycles() {
  volatile double x = 0.0;
  for (int index = 0; index < 10000; ++index) {
    x += std::sqrt(static_cast<double>(index));
  }
}

class BusyAction final : public Action {
public:
  BusyAction(std::string name, std::atomic<int> &exec_count,
             const std::chrono::milliseconds &duration)
      : Action(std::move(name)), exec_count_(exec_count), duration_(duration) {}

  void Execute() override {
    auto start = steady_clock::now();
    // Busy loop for specified duration
    while (duration_cast<milliseconds>(steady_clock::now() - start) <
           duration_) {
      BurnCpuCycles();
    }
    ++exec_count_;
  }

private:
  std::atomic<int> &exec_count_;
  std::chrono::milliseconds duration_;
};

class GlobalTimerTimingMonitorStressTest : public ::testing::Test {
protected:
  static constexpr auto kExecutionDuration = 50ms;
  static constexpr auto kActionPeriod = 100ms;
  static constexpr auto kAcceptedOverrunMargin = 10ms;
  static constexpr auto kTestDuration = 10s;

  std::atomic<int> exec_count{0};
  std::atomic<int> overruns{0};
  std::atomic<int> missed_periods{0};
  std::atomic<bool> keep_running{true};
  std::vector<std::thread> stress_threads;
  std::unique_ptr<TimingMonitor<std::chrono::steady_clock>> monitored_action;
  GlobalTimer<std::chrono::steady_clock> timer;

  void StartStressTestsAsynchronously(std::chrono::milliseconds duration) {
    const auto cpu_count = std::thread::hardware_concurrency();
    ASSERT_GT(cpu_count, 1) << "Test requires at least 2 CPU cores.";
    keep_running = true;
    stress_threads.clear();
    for (int i = 0; i < cpu_count - 1; ++i) {
      stress_threads.emplace_back([this, duration]() {
        const auto start = std::chrono::steady_clock::now();
        while (keep_running.load(std::memory_order_relaxed) &&
               std::chrono::steady_clock::now() - start < duration) {
          BurnCpuCycles();
        }
      });
    }
  }

  void ScheduleAction() {
    auto on_duration_exceeded = [this]() { ++overruns; };
    auto on_trigger_miss = [this]() { ++missed_periods; };
    auto busy_action = std::make_unique<BusyAction>("busy_action", exec_count,
                                                    kExecutionDuration);
    monitored_action =
        std::make_unique<TimingMonitor<std::chrono::steady_clock>>(
            std::move(busy_action), kExecutionDuration + kAcceptedOverrunMargin,
            on_duration_exceeded, kActionPeriod + kAcceptedOverrunMargin,
            on_trigger_miss);
    timer.SetTriggerTime(kActionPeriod,
                         [this]() { monitored_action->Execute(); });
  }

  void TearDown() override {
    keep_running = false;
    for (auto &t : stress_threads) {
      t.join();
    }
  }
};

constexpr std::chrono::milliseconds
    GlobalTimerTimingMonitorStressTest::kExecutionDuration;
constexpr std::chrono::milliseconds
    GlobalTimerTimingMonitorStressTest::kActionPeriod;
constexpr std::chrono::milliseconds
    GlobalTimerTimingMonitorStressTest::kAcceptedOverrunMargin;
constexpr std::chrono::seconds
    GlobalTimerTimingMonitorStressTest::kTestDuration;

TEST_F(GlobalTimerTimingMonitorStressTest, StressTest) {
  StartStressTestsAsynchronously(kTestDuration);
  ScheduleAction();
  std::this_thread::sleep_for(kTestDuration);

  const int total_executions = exec_count.load();
  const int total_overruns = overruns.load();
  const int total_missed = missed_periods.load();
  EXPECT_EQ(total_overruns, 0)
      << "Too many duration overruns: " << total_overruns;
  EXPECT_EQ(total_missed, 0) << "Too many period misses: " << total_missed;
  EXPECT_GE(total_executions, 99) << "Too few executions: " << total_executions;
}
