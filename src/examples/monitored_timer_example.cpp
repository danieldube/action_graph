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

void RunMonitoredTimerExample() {
  ExampleSession session(std::cout,
                         "Timing monitored graph triggered by a timer",
                         R"yaml(
- trigger:
    name: monitored_job
    period: 50 milliseconds
    action:
      name: monitored_steps
      type: sequential_actions
      decorate:
        - type: timing_monitor
          duration_limit: 30 milliseconds
          expected_period: 50 milliseconds
      actions:
        - action:
            name: measured_step
            type: wait
            duration: 60 milliseconds
        - action:
            name: announce_completion
            type: log_message
            message: "Monitored sequence finished."
)yaml");

  Timer timer;
  const auto scheduled_actions =
      BuildScheduledActions(session.Configuration(), session.ActionBuilder(),
                            session.Decorator(), session.Context(), timer);
  const auto trigger_summary = DescribeCount(
      scheduled_actions.size(), "monitored trigger", "monitored triggers");
  session.Context().Log("Timer configured " + trigger_summary +
                        " with a 30 ms budget and a 50 ms cadence.");
  session.Context().Log(
      "The timing monitor will report budget overruns and missed periods.");

  const auto observation_window = 200ms;
  ObserveForDuration(session.Context(), timer, observation_window,
                     "Running monitored sequence");
}

} // namespace examples
