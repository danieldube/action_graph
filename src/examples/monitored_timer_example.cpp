// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#include "example_runners.h"

#include "example_support.h"

#include <action_graph/builder/builder.h>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>

namespace examples {

void RunMonitoredTimerExample() {
  ExampleSession session(std::cout,
                         "Timing monitored graph triggered by a timer",
                         R"yaml(
- trigger:
    name: monitored_job
    period: 50 milliseconds
    action:
      name: monitored_sequence
      type: monitored_action
      monitor:
        duration_limit: 30 milliseconds
        period: 50 milliseconds
        on_duration_exceeded: "Execution exceeded the 30 ms budget."
        on_trigger_miss: "Trigger period was missed."
      action:
        action:
          name: monitored_steps
          type: sequential_actions
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
  const auto scheduled_actions = action_graph::builder::BuildActionGraph(
      session.Configuration(), session.Builder(), timer);
  const auto trigger_summary = DescribeCount(
      scheduled_actions.size(), "monitored action", "monitored actions");
  session.Context().Log("Timer configured " + trigger_summary +
                        " with a 30 ms budget and a 50 ms cadence.");
  session.Context().Log(
      "The monitor will report budget overruns and missed periods.");

  const auto observation_window = std::chrono::milliseconds{200};
  ObserveForDuration(session.Context(), timer, observation_window,
                     "Running monitored sequence");
}

} // namespace examples
