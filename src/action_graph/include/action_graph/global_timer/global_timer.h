// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.
#ifndef SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_GLOBAL_TIMER_GLOBAL_TIMER_H_
#define SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_GLOBAL_TIMER_GLOBAL_TIMER_H_

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <vector>

#include <action_graph/global_timer/trigger.h>

namespace action_graph {

template <typename Clock> class JumpToPastDetector {
public:
  using TimePoint = typename Clock::time_point;
  explicit JumpToPastDetector(TimePoint now,
                              std::function<void(TimePoint)> on_jump_callback)
      : on_jump_callback_(std::move(on_jump_callback)),
        recent_time_point_(now) {}

  void CallbackIfRequired(TimePoint now) {
    if (now < recent_time_point_) {
      on_jump_callback_(now);
    }
    recent_time_point_ = now;
  }

private:
  TimePoint recent_time_point_;
  std::function<void(TimePoint)> on_jump_callback_;
};

template <typename Clock> class GlobalTimer {
public:
  using Duration = typename Clock::duration;
  using TimePoint = typename Clock::time_point;

  GlobalTimer() {
    timer_thread_ = std::thread([this]() { TriggerLoop(); });
  };

  ~GlobalTimer() {
    is_timer_thread_running_ = false;
    if (timer_thread_.joinable())
      timer_thread_.join();
  };

  void SetTriggerTime(Duration period, std::function<void()> callback) {
    std::lock_guard<std::mutex> lock(schedule_mutex_);
    const auto now = Clock::now();
    auto next_trigger_time_point = now + period;
    schedule_.emplace_back(period, std::move(callback),
                           std::move(next_trigger_time_point));
  }

  void WaitOneCycle() {
    if (!is_timer_thread_running_)
      throw std::logic_error("GlobalTimer is not running.");
    // locking the mutex makes sure, that the currently running loop is
    // finished before we wait for the next cycle
    std::unique_lock<std::mutex> lock(schedule_mutex_);
    // There is a tiny chance, that the first
    // conditional_variable.notify_all() is called from the timer loop which
    // was already executed before we called WaitOneCycle(). Therefore, we
    // wait until two loops are finished.
    loop_conditional_variable_.wait(lock);
    loop_conditional_variable_.wait(lock);
    for (auto &scheduled_trigger : schedule_) {
      scheduled_trigger.trigger.WaitUntilTriggerIsFinished();
    }
  }

private:
  struct ScheduledTrigger {
    ScheduledTrigger(Duration period, std::function<void()> callback,
                     TimePoint next_trigger_time_point)
        : period(std::move(period)), trigger(std::move(callback)),
          next_trigger_time_point(std::move(next_trigger_time_point)) {}
    const Duration period;
    Trigger trigger;
    TimePoint next_trigger_time_point;
  };

  void TriggerLoop() {
    JumpToPastDetector<Clock> jump_detector(
        Clock::now(),
        [this](const TimePoint &now) { HandleClockJumpBackwards(now); });

    while (is_timer_thread_running_) {
      const auto now = Clock::now();
      jump_detector.CallbackIfRequired(now);
      TriggerIfReached(now);
      loop_conditional_variable_.notify_all();
    }
  }

  void TriggerIfReached(const TimePoint &now) {
    std::lock_guard<std::mutex> lock(schedule_mutex_);
    for (auto &trigger : schedule_) {
      if (now >= trigger.next_trigger_time_point) {
        trigger.trigger.TriggerAsynchronously();
        trigger.next_trigger_time_point += trigger.period;
      }
    }
  }

  void HandleClockJumpBackwards(const TimePoint &now) {
    std::lock_guard<std::mutex> lock(schedule_mutex_);
    for (auto &trigger : schedule_) {
      trigger.next_trigger_time_point = now + trigger.period;
    }
  }

  std::thread timer_thread_;
  std::atomic<bool> is_timer_thread_running_{true};
  std::mutex schedule_mutex_{};
  std::condition_variable loop_conditional_variable_{};
  std::vector<ScheduledTrigger> schedule_{};
};
} // namespace action_graph

#endif // SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_GLOBAL_TIMER_GLOBAL_TIMER_H_
