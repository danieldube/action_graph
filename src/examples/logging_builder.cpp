// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#include "logging_builder.h"

#include "console_log.h"

#include <action_graph/builder/builder.h>
#include <action_graph/builder/parse_duration.h>
#include <action_graph/single_action.h>
#include <yaml_cpp_configuration/yaml_node.h>

#include <chrono>
#include <thread>
#include <utility>

namespace action_graph_examples {

using action_graph::builder::ActionBuilder;
using action_graph::builder::ConfigurationNode;
using action_graph::builder::GenericActionBuilder;
using action_graph::builder::GenericActionDecorator;

namespace {

GenericActionBuilder CreateBuilder(ConsoleLog &log,
                                   GenericActionDecorator decorator) {
  auto builder =
      action_graph::builder::CreateGenericActionBuilderWithDefaultActions();
  builder.SetActionDecorator(std::move(decorator));
  builder.AddBuilderFunction("log_action", [&log](const ConfigurationNode &node,
                                                  const ActionBuilder &) {
    const auto name = node.Get("name").AsString();
    const auto message = node.Get("message").AsString();

    auto delay = std::chrono::milliseconds::zero();
    if (node.HasKey("delay")) {
      const auto delay_text = node.Get("delay").AsString();
      const auto parsed_delay =
          action_graph::builder::ParseDuration(delay_text);
      delay =
          std::chrono::duration_cast<std::chrono::milliseconds>(parsed_delay);
    }

    return std::make_unique<action_graph::SingleAction>(
        name, [name, message, delay, &log]() {
          if (delay > std::chrono::milliseconds::zero()) {
            std::this_thread::sleep_for(delay);
          }
          log.LogMessage(name + ": " + message);
        });
  });
  return builder;
}

} // namespace

GenericActionBuilder CreateLoggingActionBuilder(ConsoleLog &log) {
  return CreateBuilder(log, GenericActionDecorator{});
}

GenericActionBuilder
CreateLoggingActionBuilder(ConsoleLog &log, GenericActionDecorator decorator) {
  return CreateBuilder(log, std::move(decorator));
}

} // namespace action_graph_examples
