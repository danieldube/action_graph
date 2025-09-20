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

void RunThreeActionsTenMillisecondsExample() {
  ExampleContext context(std::cout);
  constexpr char kExampleTitle[] =
      "Three actions triggered every 10 milliseconds";
  context.Log(std::string{"\n=== "} + kExampleTitle + " ===");

  constexpr char kConfigurationText[] = R"yaml(
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

  const auto configuration =
      action_graph::yaml_cpp_configuration::Node::CreateFromString(
          kConfigurationText);
  auto builder = CreateExampleActionBuilder(context);

  Timer timer;
  const auto scheduled_actions =
      action_graph::builder::BuildActionGraph(configuration, builder, timer);
  (void)scheduled_actions;

  const auto observation_window = std::chrono::milliseconds{120};
  context.Log("Collecting high-frequency events for " +
              context.DescribeDuration(observation_window) + "...");
  std::this_thread::sleep_for(observation_window);
  timer.WaitOneCycle();

  context.PrintSummary(kExampleTitle);
}

} // namespace examples
