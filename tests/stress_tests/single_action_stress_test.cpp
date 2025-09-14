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
#include <pstl/glue_execution_defs.h>
#include <thread>
#include <vector>

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

TEST(GlobalTimerTimingMonitor, StressTest) {
  const auto cpu_count = std::thread::hardware_concurrency();
  ASSERT_GT(cpu_count, 1) << "Test requires at least 2 CPU cores.";

  // Keep (cpu_count - 1) threads busy
  std::atomic<bool> keep_running{true};
  std::vector<std::thread> stress_threads;
  for (int i = 0; i < cpu_count - 1; ++i) {
    stress_threads.emplace_back([&keep_running]() {
      while (keep_running.load(std::memory_order_relaxed)) {
        BurnCpuCycles();
      }
    });
  }

  std::atomic<int> exec_count{0};
  std::atomic<int> overruns{0};
  std::atomic<int> missed_periods{0};

  auto on_duration_exceeded = [&overruns]() { ++overruns; };
  auto on_trigger_miss = [&missed_periods]() { ++missed_periods; };

  constexpr auto kExecutionDuration = 50ms;
  constexpr auto kActionPeriod = 100ms;
  constexpr auto kAcceptedOverrunMargin = 10ms;

  auto busy_action = std::make_unique<BusyAction>("busy_action", exec_count,
                                                  kExecutionDuration);
  using Clock = std::chrono::steady_clock;
  auto monitored_action = std::make_unique<TimingMonitor<Clock>>(
      std::move(busy_action),
      kExecutionDuration + kAcceptedOverrunMargin, // duration limit
      on_duration_exceeded,
      kActionPeriod + kAcceptedOverrunMargin, // period
      on_trigger_miss);

  // Create GlobalTimer and schedule the monitored action
  GlobalTimer<Clock> timer;
  timer.SetTriggerTime(kActionPeriod,
                       [monitored_action_ptr = monitored_action.get()]() {
                         monitored_action_ptr->Execute();
                       });

  constexpr auto kTestDuration = 10s;
  std::this_thread::sleep_for(kTestDuration);

  keep_running = false;
  for (auto &t : stress_threads) {
    t.join();
  }

  int total_executions = exec_count.load();
  int total_overruns = overruns.load();
  int total_missed = missed_periods.load();

  EXPECT_LE(total_overruns, 0)
      << "Too many duration overruns: " << total_overruns;
  EXPECT_LE(total_missed, 0) << "Too many period misses: " << total_missed;
  EXPECT_GE(total_executions, 99) << "Too few executions: " << total_executions;
}
