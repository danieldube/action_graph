// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#include "examples.h"

#include "logging_builder.h"
#include "timer_helpers.h"

#include <action_graph/builder/builder.h>
#include <action_graph/builder/generic_action_decorator.h>
#include <yaml_cpp_configuration/yaml_node.h>

#include <chrono>
#include <memory>

namespace action_graph_examples {

using action_graph::GlobalTimer;
using action_graph::builder::ActionObject;
using action_graph::builder::BuildActionGraph;
using action_graph::builder::ConfigurationNode;
using action_graph::builder::DecorateWithTimingMonitor;
using action_graph::builder::GenericActionDecorator;
using action_graph::yaml_cpp_configuration::Node;

void RunMonitoredGraph(ConsoleLog &log) {
  log.LogMessage(
      "=== Example: timer-driven graph monitored by TimingMonitor ===");
  const std::string kYaml = R"(
- trigger:
    name: monitored_cycle
    period: 200 milliseconds
    action:
      name: monitored_sequence
      type: sequential_actions
      decorate:
        - type: timing_monitor
          duration_limit: 120 milliseconds
          expected_period: 200 milliseconds
      actions:
        - action:
            name: start_cycle
            type: log_action
            message: "start cycle"
        - action:
            name: slow_work
            type: log_action
            message: "simulate load"
            delay: 250 milliseconds
        - action:
            name: finish_cycle
            type: log_action
            message: "finish cycle"
)";

  GenericActionDecorator decorator{};
  decorator.AddDecoratorFunction(
      "timing_monitor",
      [&log](const ConfigurationNode &node, ActionObject action) {
        return DecorateWithTimingMonitor<TimerClock>(node, std::move(action),
                                                     log);
      });

  auto configuration = Node::CreateFromString(kYaml);
  auto builder = CreateLoggingActionBuilder(log, std::move(decorator));
  auto timer = std::make_unique<GlobalTimer<TimerClock>>();
  const auto actions = BuildActionGraph(configuration, builder, *timer);
  log.LogMessage("Registered " + std::to_string(actions.size()) +
                 " monitored action.");
  RunTimerFor(*timer, std::chrono::milliseconds(700));
  timer.reset();
}

} // namespace action_graph_examples
