// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#include <action_graph/action.h>
#include <action_graph/decorators/timing_monitor.h>
#include <action_graph/global_timer/global_timer.h>
#include <action_graph/statistics/online_distribution_estimator.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <gtest/gtest.h>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

using namespace std::chrono;
using action_graph::Action;
using action_graph::GlobalTimer;
using action_graph::decorators::TimingMonitor;
using action_graph::statistics::OnlineDistributionEstimator;

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
  static constexpr double kExpectedIntervalMs =
      std::chrono::duration<double, std::milli>(kActionPeriod).count();
  static constexpr double kExpectedFrequencyHz =
      1.0 / std::chrono::duration<double>(kActionPeriod).count();
  static constexpr double kMaxAllowedJitterMeanMs = 3.0;
  static constexpr double kMaxAllowedJitterStdDevMs = 15.0;
  static constexpr double kMaxAllowedFrequencyMeanHz = 2.0;
  static constexpr double kMaxAllowedFrequencyStdDevHz = 5.0;
  static constexpr double kMaxAllowedFrequencyDifferenceHz = 10.0;
  static constexpr int kMaxAllowedDurationOverruns = 2;

  std::atomic<int> exec_count{0};
  std::atomic<int> overruns{0};
  std::atomic<int> missed_periods{0};
  std::atomic<bool> keep_running{true};
  std::atomic<bool> collect_metrics{true};
  std::vector<std::thread> stress_threads;
  std::unique_ptr<TimingMonitor<std::chrono::steady_clock>> monitored_action;
  GlobalTimer<std::chrono::steady_clock> timer;
  std::mutex metrics_mutex_{};
  OnlineDistributionEstimator<double> jitter_distribution_{};
  OnlineDistributionEstimator<double> frequency_difference_distribution_{};
  double max_jitter_difference_ms_{0.0};
  double max_frequency_difference_hz_{0.0};
  std::chrono::steady_clock::time_point last_execution_time_{};
  bool has_last_execution_time_{false};

  void StartStressTestsAsynchronously(std::chrono::milliseconds duration) {
    const auto cpu_count = std::thread::hardware_concurrency();
    ASSERT_GT(cpu_count, 1) << "Test requires at least 2 CPU cores.";
    keep_running.store(true, std::memory_order_relaxed);
    collect_metrics.store(true, std::memory_order_relaxed);
    stress_threads.clear();
    const auto stress_thread_count =
        cpu_count > 2 ? static_cast<int>(cpu_count - 2) : 1;
    for (int i = 0; i < stress_thread_count; ++i) {
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
    {
      std::lock_guard<std::mutex> lock(metrics_mutex_);
      jitter_distribution_ = OnlineDistributionEstimator<double>();
      frequency_difference_distribution_ = OnlineDistributionEstimator<double>();
      max_jitter_difference_ms_ = 0.0;
      max_frequency_difference_hz_ = 0.0;
      last_execution_time_ = {};
      has_last_execution_time_ = false;
    }
    timer.SetTriggerTime(kActionPeriod, [this]() {
      const auto now = steady_clock::now();
      if (!collect_metrics.load(std::memory_order_relaxed)) {
        monitored_action->Execute();
        return;
      }

      {
        std::lock_guard<std::mutex> lock(metrics_mutex_);
        if (!collect_metrics.load(std::memory_order_relaxed)) {
          last_execution_time_ = now;
          has_last_execution_time_ = true;
        } else {
          if (has_last_execution_time_) {
            const auto interval = now - last_execution_time_;
            const double interval_ms =
                std::chrono::duration<double, std::milli>(interval).count();
            const double jitter_ms = interval_ms - kExpectedIntervalMs;
            jitter_distribution_.AddValue(jitter_ms);
            max_jitter_difference_ms_ =
                std::max(max_jitter_difference_ms_, std::abs(jitter_ms));

            const double interval_seconds =
                std::chrono::duration<double>(interval).count();
            if (interval_seconds > 0.0) {
              const double frequency_hz = 1.0 / interval_seconds;
              const double frequency_difference =
                  frequency_hz - kExpectedFrequencyHz;
              frequency_difference_distribution_.AddValue(
                  frequency_difference);
              max_frequency_difference_hz_ = std::max(
                  max_frequency_difference_hz_,
                  std::abs(frequency_difference));
            }
          }
          last_execution_time_ = now;
          has_last_execution_time_ = true;
        }
      }

      monitored_action->Execute();
    });
  }

  void TearDown() override {
    keep_running.store(false, std::memory_order_relaxed);
    collect_metrics.store(false, std::memory_order_relaxed);
    for (auto &t : stress_threads) {
      t.join();
    }
  }
};

TEST_F(GlobalTimerTimingMonitorStressTest, StressTest) {
  StartStressTestsAsynchronously(kTestDuration);
  ScheduleAction();
  std::this_thread::sleep_for(kTestDuration);
  collect_metrics.store(false, std::memory_order_relaxed);
  std::this_thread::sleep_for(kActionPeriod);

  OnlineDistributionEstimator<double>::NormalDistributionParameters
      jitter_distribution{};
  OnlineDistributionEstimator<double>::NormalDistributionParameters
      frequency_distribution{};
  double max_jitter_difference = 0.0;
  double max_frequency_difference = 0.0;

  {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    ASSERT_GT(jitter_distribution_.GetSampleSize(), 0u);
    ASSERT_GT(frequency_difference_distribution_.GetSampleSize(), 0u);
    jitter_distribution = jitter_distribution_.GetNormalDistribution();
    frequency_distribution =
        frequency_difference_distribution_.GetNormalDistribution();
    max_jitter_difference = max_jitter_difference_ms_;
    max_frequency_difference = max_frequency_difference_hz_;
  }

  const int total_executions = exec_count.load();
  const int total_overruns = overruns.load();
  const int total_missed = missed_periods.load();
  EXPECT_LE(total_overruns, kMaxAllowedDurationOverruns)
      << "Too many duration overruns: " << total_overruns;
  EXPECT_LE(total_missed, 15)
      << "Too many period misses: " << total_missed;
  EXPECT_GE(total_executions, 99) << "Too few executions: " << total_executions;

  const double jitter_mean_abs = std::abs(jitter_distribution.mean);
  EXPECT_LT(jitter_mean_abs, kMaxAllowedJitterMeanMs)
      << "Average jitter too high: " << jitter_distribution.mean << " ms";
  EXPECT_LT(jitter_distribution.standard_deviation, kMaxAllowedJitterStdDevMs)
      << "Jitter standard deviation too high: "
      << jitter_distribution.standard_deviation << " ms";
  const double max_allowed_jitter_difference =
      std::chrono::duration<double, std::milli>(kAcceptedOverrunMargin)
          .count() * 6.0;
  EXPECT_LT(max_jitter_difference, max_allowed_jitter_difference)
      << "Maximum jitter difference too high: " << max_jitter_difference
      << " ms";

  const double frequency_mean_abs = std::abs(frequency_distribution.mean);
  EXPECT_LT(frequency_mean_abs, kMaxAllowedFrequencyMeanHz)
      << "Average frequency deviation too high: "
      << frequency_distribution.mean << " Hz";
  EXPECT_LT(frequency_distribution.standard_deviation,
            kMaxAllowedFrequencyStdDevHz)
      << "Frequency standard deviation too high: "
      << frequency_distribution.standard_deviation << " Hz";
  EXPECT_LT(max_frequency_difference, kMaxAllowedFrequencyDifferenceHz)
      << "Maximum frequency deviation too high: " << max_frequency_difference
      << " Hz";
}
