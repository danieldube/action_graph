// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#include "example_runners.h"
#include "example_timer_runner.h"

#include <string_view>

namespace {
constexpr std::string_view kTitle =
    "Sequential and parallel actions executed once";
constexpr std::string_view kConfiguration = R"(
action:
  name: orchestrate_data_flow
  type: sequential_actions
  actions:
    - action:
        name: announce_start
        type: log_message
        message: "Announcing a composed action graph."
    - action:
        name: fetch_inputs
        type: parallel_actions
        actions:
          - action:
              name: pull_profiles
              type: wait_and_log
              message: "Retrieving customer profiles"
              duration: 30 milliseconds
          - action:
              name: pull_orders
              type: wait_and_log
              message: "Collecting recent orders"
              duration: 40 milliseconds
    - action:
        name: aggregate_results
        type: log_message
        message: "Aggregated results ready for presentation."
)";
} // namespace

void RunGraphExecutionExample() { RunGraphExample(kTitle, kConfiguration); }
