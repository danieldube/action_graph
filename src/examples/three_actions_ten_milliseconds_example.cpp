// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#include "example_runners.h"

#include "example_support.h"

#include <chrono>
#include <iostream>
#include <string>

using namespace std::chrono_literals;

namespace examples {

void RunThreeActionsTenMillisecondsExample() {
  ExampleSession session(std::cout,
                         "Three actions triggered every 10 milliseconds",
                         R"yaml(
- trigger:
    name: alpha
    period: 10 milliseconds
    action:
      name: alpha_action
      type: log_message
      message: "Alpha fired."
- trigger:
    name: beta
    period: 10 milliseconds
    action:
      name: beta_action
      type: log_message
      message: "Beta fired."
- trigger:
    name: gamma
    period: 10 milliseconds
    action:
      name: gamma_action
      type: log_message
      message: "Gamma fired."
)yaml");

  Timer timer;
  const auto scheduled_actions =
      BuildScheduledActions(session.Configuration(), session.ActionBuilder(),
                            session.Decorator(), session.Context(), timer);
  const auto trigger_summary =
      DescribeCount(scheduled_actions.size(), "action", "actions");
  session.Context().Log("Timer configured " + trigger_summary +
                        " that repeat every 10 milliseconds.");

  const auto observation_window = 120ms;
  ObserveForDuration(session.Context(), timer, observation_window,
                     "Collecting high-frequency events");
}

} // namespace examples
