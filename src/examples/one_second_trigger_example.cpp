// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#include "example_runners.h"
#include "example_timer_runner.h"

#include <string_view>

namespace {
constexpr std::string_view kTitle = "One action every second";
constexpr std::string_view kConfiguration = R"(
- trigger:
    name: heartbeat
    period: 1 seconds
    action:
      name: narrate_heartbeat
      type: log_message
      message: "A gentle reminder that another second passed."
)";
constexpr int kCycles = 3;
} // namespace

void RunOneSecondTriggerExample() {
  RunTimedExample(kTitle, kConfiguration, kCycles);
}
