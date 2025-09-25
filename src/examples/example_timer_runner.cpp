// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#include "example_timer_runner.h"

#include <action_graph/builder/builder.h>
#include <action_graph/builder/parse_duration.h>
#include <action_graph/global_timer/global_timer.h>
#include <yaml_cpp_configuration/yaml_node.h>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "example_action_builder.h"
#include "example_context.h"

namespace {
using SteadyTimer = action_graph::GlobalTimer<std::chrono::steady_clock>;
using ConfigurationNode = action_graph::builder::ConfigurationNode;

std::chrono::nanoseconds
FindLongestPeriod(const ConfigurationNode &configuration) {
  std::chrono::nanoseconds longest_period{0};
  for (std::size_t index = 0; index < configuration.Size(); ++index) {
    const auto &entry = configuration.Get(index);
    if (!entry.HasKey("trigger")) {
      continue;
    }
    const auto &trigger = entry.Get("trigger");
    const auto period_text = trigger.Get("period").AsString();
    const auto period = action_graph::builder::ParseDuration(period_text);
    const auto as_nanoseconds =
        std::chrono::duration_cast<std::chrono::nanoseconds>(period);
    longest_period = std::max(longest_period, as_nanoseconds);
  }
  return longest_period;
}

void PrintConfiguration(std::string_view yaml_text) {
  std::cout << "Configuration:\n" << yaml_text << '\n';
}
} // namespace

void RunTimedExample(std::string_view title, std::string_view yaml_text,
                     int cycles) {
  std::cout << "\n=== " << title << " ===\n";
  PrintConfiguration(yaml_text);

  ExampleContext context{std::cout};
  ExampleActionBuilder action_builder{context};
  auto configuration =
      action_graph::yaml_cpp_configuration::Node::CreateFromString(
          std::string{yaml_text});
  const auto wait_period = FindLongestPeriod(configuration);
  SteadyTimer timer;
  auto scheduled_actions = action_graph::builder::BuildActionGraph(
      configuration, action_builder, timer);

  for (int cycle = 0; cycle < cycles; ++cycle) {
    if (wait_period != std::chrono::nanoseconds::zero()) {
      std::this_thread::sleep_for(wait_period);
    }
    timer.WaitOneCycle();
  }

  static_cast<void>(scheduled_actions);

  context.PrintSummary(title);
}

void RunGraphExample(std::string_view title, std::string_view yaml_text) {
  std::cout << "\n=== " << title << " ===\n";
  PrintConfiguration(yaml_text);

  ExampleContext context{std::cout};
  ExampleActionBuilder action_builder{context};
  const auto configuration =
      action_graph::yaml_cpp_configuration::Node::CreateFromString(
          std::string{yaml_text});
  auto action = action_builder(configuration);
  action->Execute();

  context.PrintSummary(title);
}
