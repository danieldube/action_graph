// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#include "example_runners.h"

#include "example_support.h"

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

namespace examples {

void RunOneSecondTriggerExample() {
  ExampleSession session(std::cout, "One action triggered every second",
                         R"yaml(
- trigger:
    name: heartbeat
    period: 1 seconds
    action:
      name: heartbeat_action
      type: log_message
      message: "Heartbeat action executed."
)yaml");

  Timer timer;
  const auto scheduled_actions =
      BuildScheduledActions(session.Configuration(), session.Builder(), timer);
  const auto trigger_summary =
      DescribeCount(scheduled_actions.size(), "action", "actions");
  session.Context().Log("Timer configured " + trigger_summary +
                        " to fire once per second.");

  const auto observation_window = std::chrono::seconds{4};
  ObserveForDuration(session.Context(), timer, observation_window,
                     "Observing heartbeat");
}

} // namespace examples
