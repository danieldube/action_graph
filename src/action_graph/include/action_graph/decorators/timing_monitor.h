// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#ifndef ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_DECORATORS_TIMING_MONITOR_H_
#define ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_DECORATORS_TIMING_MONITOR_H_

#include <action_graph/decorators/decorated_action.h>
#include <chrono>
#include <functional>
#include <memory>

namespace action_graph {
namespace decorators {

template <typename Clock> class TimingMonitor : public DecoratedAction {
public:
  using Duration = typename Clock::duration;
  using TimePoint = typename Clock::time_point;
  using Callback = std::function<void()>;

  TimingMonitor(std::unique_ptr<action_graph::Action> action,
                Duration duration_limit, Callback on_duration_exceeded,
                Duration period, Callback on_trigger_miss)
      : DecoratedAction(std::move(action)), duration_limit_(duration_limit),
        on_duration_exceeded_(std::move(on_duration_exceeded)), period_(period),
        on_trigger_miss_(std::move(on_trigger_miss)) {}

  void Execute() override {
    CheckTriggerMiss();
    const auto start = Clock::now();
    GetAction().Execute();
    const auto duration = Clock::now() - start;
    if (duration > duration_limit_) {
      on_duration_exceeded_();
    }
  }

private:
  void CheckTriggerMiss() {
    const auto now = Clock::now();
    static const Duration kAcceptableDelay = period_;
    if (now - last_execution_time_ > kAcceptableDelay) {
      on_trigger_miss_();
    }
    last_execution_time_ = now;
  }

  Duration duration_limit_;
  Duration period_{};
  TimePoint last_execution_time_{Clock::now()};
  Callback on_duration_exceeded_;
  Callback on_trigger_miss_;
};
} // namespace decorators
} // namespace action_graph

#endif // ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_DECORATORS_TIMING_MONITOR_H_
