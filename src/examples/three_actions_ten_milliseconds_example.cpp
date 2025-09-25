// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#include "example_runners.h"
#include "example_timer_runner.h"

#include <string_view>

namespace {
constexpr std::string_view kTitle = "Three actions every ten milliseconds";
constexpr std::string_view kConfiguration = R"(
- trigger:
    name: telemetry_burst
    period: 10 milliseconds
    action:
      name: fan_out_samples
      type: parallel_actions
      actions:
        - action:
            name: log_left_sensor
            type: log_message
            message: "Left sensor captured a sample."
        - action:
            name: log_right_sensor
            type: log_message
            message: "Right sensor captured a sample."
        - action:
            name: log_central_unit
            type: log_message
            message: "Central unit correlated the readings."
)";
constexpr int kCycles = 10;
} // namespace

void RunThreeActionsTenMillisecondsExample() {
  RunTimedExample(kTitle, kConfiguration, kCycles);
}
