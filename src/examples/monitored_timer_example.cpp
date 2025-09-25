// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#include "example_runners.h"
#include "example_timer_runner.h"

#include <string_view>

namespace {
constexpr std::string_view kTitle =
    "Timer-driven graph monitored with a timing decorator";
constexpr std::string_view kConfiguration = R"(
- trigger:
    name: monitored_pipeline
    period: 50 milliseconds
    action:
      name: monitored_sequence
      type: sequential_actions
      decorate:
        - type: timing_monitor
          duration_limit: 40 milliseconds
          expected_period: 50 milliseconds
      actions:
        - action:
            name: capture_measurement
            type: log_message
            message: "Captured a measurement from the probe."
        - action:
            name: publish_with_delay
            type: wait_and_log
            message: "Publishing measurement to observers"
            duration: 60 milliseconds
)";
constexpr int kCycles = 5;
} // namespace

void RunMonitoredTimerExample() {
  RunTimedExample(kTitle, kConfiguration, kCycles);
}
