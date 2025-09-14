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

using namespace std::chrono;
using action_graph::Action;
using action_graph::GlobalTimer;
using action_graph::decorators::TimingMonitor;

class BusyAction final : public Action {
public:
  BusyAction(std::string name, std::atomic<int> &exec_count)
      : Action(std::move(name)), exec_count_(exec_count) {}

  void Execute() override {
    auto start = steady_clock::now();
    // Busy loop for ~50ms
    while (duration_cast<milliseconds>(steady_clock::now() - start).count() <
           50) {
      asm volatile("");
    }
    exec_count_++;
  }

private:
  std::atomic<int> &exec_count_;
};

void BurnCpuCycles() {
  volatile double x = 0.0;
  for (int index = 0; index < 10000; ++index) {
    x += std::sqrt(static_cast<double>(index));
  }
}

TEST(GlobalTimerTimingMonitor, StressTest) {
  const int cpu_count = std::thread::hardware_concurrency();
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

  // TimingMonitor callbacks
  auto on_duration_exceeded = [&overruns]() { overruns++; };
  auto on_trigger_miss = [&missed_periods]() { missed_periods++; };

  // Create the busy action
  auto busy_action = std::make_unique<BusyAction>("busy_action", exec_count);

  // Decorate with TimingMonitor
  using Clock = std::chrono::steady_clock;
  auto monitored_action =
      std::make_unique<TimingMonitor<Clock>>(std::move(busy_action),
                                             milliseconds(60), // duration limit
                                             on_duration_exceeded,
                                             milliseconds(110), // period
                                             on_trigger_miss);

  // Create GlobalTimer and schedule the monitored action
  GlobalTimer<Clock> timer;
  timer.SetTriggerTime(milliseconds(100),
                       [monitored_action_ptr = monitored_action.get()]() {
                         monitored_action_ptr->Execute();
                       });

  // Run for 10 seconds
  std::this_thread::sleep_for(seconds(10));

  // Stop stress threads
  keep_running = false;
  for (auto &t : stress_threads) {
    t.join();
  }

  // Check results
  int total_executions = exec_count.load();
  int total_overruns = overruns.load();
  int total_missed = missed_periods.load();

  // Allow a small margin for missed deadlines due to OS scheduling
  EXPECT_LE(total_overruns, 2)
      << "Too many duration overruns: " << total_overruns;
  EXPECT_LE(total_missed, 2) << "Too many period misses: " << total_missed;
  EXPECT_GE(total_executions, 95) << "Too few executions: " << total_executions;
}
