// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#include "example_action_builder.h"

#include <action_graph/builder/parse_duration.h>
#include <action_graph/single_action.h>

#include <chrono>
#include <sstream>
#include <thread>
#include <utility>

namespace {
std::string DescribeDuration(std::chrono::nanoseconds duration) {
  using namespace std::chrono;
  if (duration >= seconds{1}) {
    return std::to_string(duration_cast<seconds>(duration).count()) + " s";
  }
  if (duration >= milliseconds{1}) {
    return std::to_string(duration_cast<milliseconds>(duration).count()) +
           " ms";
  }
  if (duration >= microseconds{1}) {
    return std::to_string(duration_cast<microseconds>(duration).count()) +
           " µs";
  }
  return std::to_string(duration.count()) + " ns";
}
} // namespace

ExampleActionBuilder::ExampleActionBuilder(ExampleContext &context)
    : context_(context),
      action_builder_(action_graph::builder::
                          CreateGenericActionBuilderWithDefaultActions()) {
  RegisterActions();
  RegisterDecorators();
}

action_graph::builder::ActionObject
ExampleActionBuilder::operator()(const ConfigurationNode &node) const {
  auto action = action_builder_(node);
  const auto &action_node = node.Get("action");
  return decorator_builder_(action_node, std::move(action));
}

void ExampleActionBuilder::RegisterActions() {
  action_builder_.AddBuilderFunction(
      "log_message", [this](const ConfigurationNode &node,
                            const action_graph::builder::ActionBuilder &) {
        return BuildLogMessageAction(node);
      });

  action_builder_.AddBuilderFunction(
      "wait_and_log", [this](const ConfigurationNode &node,
                             const action_graph::builder::ActionBuilder &) {
        return BuildWaitAndLogAction(node);
      });
}

void ExampleActionBuilder::RegisterDecorators() {
  decorator_builder_.AddDecoratorFunction(
      "timing_monitor", [this](const ConfigurationNode &node,
                               action_graph::builder::ActionObject action) {
        return action_graph::builder::DecorateWithTimingMonitor<
            std::chrono::steady_clock>(node, std::move(action), context_);
      });
}

action_graph::builder::ActionObject ExampleActionBuilder::BuildLogMessageAction(
    const ConfigurationNode &node) const {
  const auto name = node.Get("name").AsString();
  const auto message = node.Get("message").AsString();
  return std::make_unique<action_graph::SingleAction>(
      name, [this, name, message]() {
        const auto execution_index = context_.RecordExecution(name);
        std::ostringstream stream;
        stream << name << " (#" << execution_index << ") " << message;
        context_.LogMessage(stream.str());
      });
}

action_graph::builder::ActionObject ExampleActionBuilder::BuildWaitAndLogAction(
    const ConfigurationNode &node) const {
  const auto name = node.Get("name").AsString();
  const auto message = node.Get("message").AsString();
  const auto duration =
      action_graph::builder::ParseDuration(node.Get("duration").AsString());
  const auto wait_duration =
      std::chrono::duration_cast<std::chrono::nanoseconds>(duration);
  return std::make_unique<action_graph::SingleAction>(
      name, [this, name, message, wait_duration]() {
        const auto execution_index = context_.RecordExecution(name);
        std::ostringstream stream;
        stream << name << " (#" << execution_index << ") starting: " << message
               << " (" << DescribeDuration(wait_duration) << ")";
        context_.LogMessage(stream.str());
        std::this_thread::sleep_for(wait_duration);
        context_.LogMessage(name + std::string{" completed."});
      });
}
