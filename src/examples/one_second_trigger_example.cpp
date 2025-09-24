// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#include "example_runners.h"

#include "example_configurations.h"
#include "example_support.h"

#include <chrono>
#include <iostream>
#include <string>
#include <vector>

using namespace std::chrono_literals;

namespace {

constexpr std::chrono::seconds kHeartbeatObservationWindow{4};

std::vector<action_graph::builder::ActionObject>
ScheduleHeartbeat(examples::ExampleSession &session, examples::Timer &timer) {
  auto actions = examples::BuildScheduledActions(
      session.Configuration(), session.ActionBuilder(), session.Decorator(),
      session.Context(), timer);
  return actions;
}

void LogHeartbeatSummary(examples::ExampleSession &session,
                         std::size_t action_count) {
  const auto summary =
      examples::DescribeCount(action_count, "action", "actions");
  session.Context().Log("Timer configured " + summary +
                        " to fire once per second.");
}

void ObserveHeartbeat(examples::ExampleSession &session,
                      examples::Timer &timer) {
  examples::ObserveForDuration(session.Context(), timer,
                               kHeartbeatObservationWindow,
                               "Observing heartbeat");
}

} // namespace

namespace examples {

void RunOneSecondTriggerExample() {
  ExampleSession session(std::cout, "One action triggered every second",
                         configurations::OneSecondTriggerYaml());
  Timer timer;
  auto actions = ScheduleHeartbeat(session, timer);
  LogHeartbeatSummary(session, actions.size());
  ObserveHeartbeat(session, timer);
}

} // namespace examples
