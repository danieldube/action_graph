// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#include <action_graph/global_timer/trigger.h>
#include <thread>
#include <utility>

namespace action_graph {

Trigger::Trigger(std::function<void()> callback)
    : callback_(std::move(callback)) {}

Trigger::Trigger(Trigger &&other) noexcept
    : callback_(std::move(other.callback_)),
      is_running_(other.is_running_.load()) {}

Trigger::~Trigger() { WaitUntilTriggerIsFinished(); }

void Trigger::TriggerAsynchronously() {
  if (is_running_.exchange(true)) {
    return;
  }
  std::thread([this]() {
    callback_();
    is_running_ = false;
  }).detach();
}

void Trigger::WaitUntilTriggerIsFinished() const {
  while (is_running_) {
    std::this_thread::yield();
  }
}

} // namespace action_graph
