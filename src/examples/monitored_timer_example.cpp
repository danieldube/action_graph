// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#include "example_runners.h"

#include "example_support.h"

#include <chrono>
#include <iostream>
#include <string>
#include <vector>

using namespace std::chrono_literals;

namespace {

constexpr char kMonitoredTimerYaml[] = R"yaml(
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
)yaml";

constexpr std::chrono::milliseconds kMonitorObservationWindow{200};

std::vector<action_graph::builder::ActionObject>
ScheduleMonitoredActions(examples::ExampleSession &session,
                         examples::Timer &timer) {
  auto actions = examples::BuildScheduledActions(
      session.Configuration(), session.ActionBuilder(), session.Decorator(),
      session.Context(), timer);
  return actions;
}

void LogMonitoredSummary(examples::ExampleSession &session,
                         std::size_t action_count) {
  const auto summary = examples::DescribeCount(
      action_count, "monitored trigger", "monitored triggers");
  session.Context().Log("Timer configured " + summary +
                        " with a 30 ms budget and a 50 ms cadence.");
}

void DescribeTimingMonitor(examples::ExampleSession &session) {
  session.Context().Log(
      "The timing monitor will report budget overruns and missed periods.");
}

void ObserveMonitoredSequence(examples::ExampleSession &session,
                              examples::Timer &timer) {
  examples::ObserveForDuration(session.Context(), timer,
                               kMonitorObservationWindow,
                               "Running monitored sequence");
}

} // namespace

namespace examples {

void RunMonitoredTimerExample() {
  ExampleSession session(std::cout,
                         "Timing monitored graph triggered by a timer",
                         kMonitoredTimerYaml);
  Timer timer;
  auto actions = ScheduleMonitoredActions(session, timer);
  LogMonitoredSummary(session, actions.size());
  DescribeTimingMonitor(session);
  ObserveMonitoredSequence(session, timer);
}

} // namespace examples
