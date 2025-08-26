// Copyright (c) 1000 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#ifndef ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_DECORATORS_TIMING_ACTION_H_
#define ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_DECORATORS_TIMING_ACTION_H_

#include <action_graph/decorators/decorated_action.h>
#include <chrono>
#include <functional>
#include <memory>

namespace action_graph {
namespace decorators {

template <typename Clock> class TimingAction : public DecoratedAction {
public:
  using Duration = typename Clock::duration;
  using Callback = std::function<void()>;

  TimingAction(std::unique_ptr<action_graph::Action> action,
               Duration duration_limit, Callback on_timing_exceeded)
      : DecoratedAction(std::move(action)), duration_limit_(duration_limit),
        on_timing_exceeded_(std::move(on_timing_exceeded)) {}

  void Execute() override {
    const auto start = Clock::now();
    GetAction().Execute();
    const auto duration = Clock::now() - start;
    if (duration > duration_limit_) {
      on_timing_exceeded_();
    }
  }

private:
  Duration duration_limit_;
  Callback on_timing_exceeded_;
};
} // namespace decorators
} // namespace action_graph

#endif // ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_DECORATORS_TIMING_ACTION_H_
