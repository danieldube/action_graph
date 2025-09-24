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

constexpr char kHighFrequencyYaml[] = R"yaml(
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
)yaml";

constexpr std::chrono::milliseconds kBurstObservationWindow{120};

std::vector<action_graph::builder::ActionObject>
ScheduleHighFrequencyTriggers(examples::ExampleSession &session,
                              examples::Timer &timer) {
  auto actions = examples::BuildScheduledActions(
      session.Configuration(), session.ActionBuilder(), session.Decorator(),
      session.Context(), timer);
  return actions;
}

void LogHighFrequencySummary(examples::ExampleSession &session,
                             std::size_t action_count) {
  const auto summary =
      examples::DescribeCount(action_count, "action", "actions");
  session.Context().Log("Timer configured " + summary +
                        " that repeat every 10 milliseconds.");
}

void ObserveHighFrequency(examples::ExampleSession &session,
                          examples::Timer &timer) {
  examples::ObserveForDuration(session.Context(), timer,
                               kBurstObservationWindow,
                               "Collecting high-frequency events");
}

} // namespace

namespace examples {

void RunThreeActionsTenMillisecondsExample() {
  ExampleSession session(std::cout,
                         "Three actions triggered every 10 milliseconds",
                         kHighFrequencyYaml);
  Timer timer;
  auto actions = ScheduleHighFrequencyTriggers(session, timer);
  LogHighFrequencySummary(session, actions.size());
  ObserveHighFrequency(session, timer);
}

} // namespace examples
