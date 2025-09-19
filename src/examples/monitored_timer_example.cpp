// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#include "example_runners.h"

#include "example_support.h"

#include <action_graph/builder/builder.h>
#include <yaml_cpp_configuration/yaml_node.h>

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

namespace examples {

void RunMonitoredTimerExample() {
  ExampleContext context(std::cout);
  constexpr char kExampleTitle[] =
      "Timing monitored graph triggered by a timer";
  context.Log(std::string{"\n=== "} + kExampleTitle + " ===");

  constexpr char kConfigurationText[] = R"yaml(
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
)yaml";

  const auto configuration =
      action_graph::yaml_cpp_configuration::Node::CreateFromString(
          kConfigurationText);
  auto builder = CreateExampleActionBuilder(context);

  Timer timer;
  const auto scheduled_actions =
      action_graph::builder::BuildActionGraph(configuration, builder, timer);
  (void)scheduled_actions;

  const auto observation_window = std::chrono::milliseconds{200};
  context.Log("Running monitored sequence for approximately " +
              context.DescribeDuration(observation_window) + "...");
  std::this_thread::sleep_for(observation_window);
  timer.WaitOneCycle();

  context.PrintSummary(kExampleTitle);
}

} // namespace examples
