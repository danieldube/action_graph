// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#include <action_graph/builder/generic_action_decorator.h>
#include <action_graph/decorators/timing_monitor.h>
#include <action_graph/log.h>

namespace action_graph {
namespace builder {

ActionObject DecorateAction(const ConfigurationNode &node, ActionObject action,
                            const DecorateFunctions &decorate_functions) {
  if (!node.HasKey("type")) {
    throw ConfigurationError("Decorator type is not defined.", node);
  }
  auto decorator_type = node.Get("type").AsString();
  auto decorator = decorate_functions.find(decorator_type);
  if (decorator == decorate_functions.end()) {
    throw BuildError("No decorator defined for " + decorator_type + ".");
  }
  const auto &decorate_function = decorator->second;
  return decorate_function(node, std::move(action));
}

ActionObject GenericActionDecorator::operator()(const ConfigurationNode &node,
                                                ActionObject action) const {
  if (!node.HasKey("decorate"))
    return action;
  const auto &decorators_node = node.Get("decorate");

  for (size_t decorator_index = 0; decorator_index < decorators_node.Size();
       ++decorator_index) {
    const auto &decorator_node = decorators_node.Get(decorator_index);
    action =
        DecorateAction(decorator_node, std::move(action), decorate_functions_);
  }
  return action;
}

void GenericActionDecorator::AddDecoratorFunction(
    const std::string &action_type, DecorateFunction decorate_function) {
  decorate_functions_[action_type] = std::move(decorate_function);
}

std::chrono::duration<double>
GetDurationFromConfigurationNode(const ConfigurationNode &node,
                                 const std::string &name) {
  if (!node.HasKey(name))
    throw ConfigurationError("The value " + name + " is not defined.", node);
  auto text = node.Get(name).AsString();
  auto duration = ParseDuration(text);
  return duration;
}

template <typename Clock>
ActionObject DecorateWithTimingMonitor(const ConfigurationNode &node,
                                       ActionObject action,
                                       action_graph::Log &log) {
  using TimingMonitor = action_graph::decorators::TimingMonitor<Clock>;
  auto duration_limit =
      GetDurationFromConfigurationNode(node, "duration_limit");
  auto expected_period =
      GetDurationFromConfigurationNode(node, "expected_period");
  const std::string action_name = action->name;
  return TimingMonitor(
      action, duration_limit,
      [&log, action_name]() {
        log.LogError("Duration for action " + action_name +
                     " exceeded the limit.");
      },
      expected_period,
      [&log, action_name]() {
        log.LogError("The period for action " + action_name +
                     " exceeded the limit.");
      });
}

} // namespace builder
} // namespace action_graph
