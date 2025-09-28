// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#include "examples.h"

#include "logging_builder.h"
#include "timer_helpers.h"

#include <action_graph/builder/builder.h>
#include <yaml_cpp_configuration/yaml_node.h>

#include <chrono>
#include <memory>

namespace action_graph_examples {

using action_graph::GlobalTimer;
using action_graph::builder::BuildActionGraph;
using action_graph::yaml_cpp_configuration::Node;

void RunSingleSecondTriggerExample(ConsoleLog &log) {
  log.LogMessage("=== Example: one action triggered every second ===");
  const std::string kYaml = R"(
- trigger:
    name: announce_tick
    period: 1 seconds
    action:
      name: log_tick
      type: log_action
      message: "tick"
)";

  auto configuration = Node::CreateFromString(kYaml);
  auto builder = CreateLoggingActionBuilder(log);
  auto timer = std::make_unique<GlobalTimer<TimerClock>>();
  const auto actions = BuildActionGraph(configuration, builder, *timer);
  log.LogMessage("Registered " + std::to_string(actions.size()) +
                 " periodic action.");
  RunTimerFor(*timer, std::chrono::seconds(3));
  timer.reset();
}

} // namespace action_graph_examples
