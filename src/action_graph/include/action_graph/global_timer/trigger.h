// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#ifndef ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_GLOBAL_TIMER_TRIGGER_H_
#define ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_GLOBAL_TIMER_TRIGGER_H_

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <stdexcept>
#include <thread>
namespace action_graph {

class Trigger {
public:
  explicit Trigger(std::function<void()> callback);

  Trigger(const Trigger &) = delete;
  Trigger(Trigger &&other) noexcept;
  Trigger &operator=(const Trigger &) = delete;
  Trigger &operator=(Trigger &&) = delete;

  ~Trigger();

  void TriggerAsynchronously();
  void WaitUntilTriggerIsFinished() const;

private:
  std::function<void()> callback_;
  std::atomic<bool> is_running_{false};
};

} // namespace action_graph

#endif // ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_GLOBAL_TIMER_TRIGGER_H_
