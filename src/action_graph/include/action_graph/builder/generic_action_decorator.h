// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.
#ifndef SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_BUILDER_GENERIC_ACTION_DECORATOR_H_
#define SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_BUILDER_GENERIC_ACTION_DECORATOR_H_

#include "builder.h"
#include <action_graph/action.h>
#include <action_graph/builder/configuration_node.h>
#include <action_graph/builder/parse_duration.h>
#include <action_graph/decorators/decorated_action.h>
#include <action_graph/decorators/timing_monitor.h>
#include <action_graph/log.h>
#include <functional>
#include <map>
#include <string>

namespace action_graph {
namespace builder {

using DecorateFunction =
    std::function<ActionObject(const ConfigurationNode &, ActionObject action)>;
using DecorateFunctions = std::map<std::string, DecorateFunction>;

class GenericActionDecorator {
public:
  GenericActionDecorator() = default;
  ActionObject operator()(const ConfigurationNode &node,
                          ActionObject action) const;
  void AddDecoratorFunction(const std::string &action_type,
                            DecorateFunction decorate_function);

private:
  DecorateFunctions decorate_functions_;
};

template <typename Clock>
typename Clock::duration
GetDurationFromConfigurationNode(const ConfigurationNode &node,
                                 const std::string &name) {
  if (!node.HasKey(name))
    throw ConfigurationError("The value " + name + " is not defined.", node);
  const auto text = node.Get(name).AsString();
  auto duration = ParseDuration(text);
  return std::chrono::duration_cast<typename Clock::duration>(duration);
}

template <typename Clock>
ActionObject DecorateWithTimingMonitor(const ConfigurationNode &node,
                                       ActionObject action,
                                       action_graph::Log &log) {
  using TimingMonitor = action_graph::decorators::TimingMonitor<Clock>;
  auto duration_limit =
      GetDurationFromConfigurationNode<Clock>(node, "duration_limit");
  auto expected_period =
      GetDurationFromConfigurationNode<Clock>(node, "expected_period");
  const std::string action_name = action->name;
  return std::make_unique<TimingMonitor>(
      std::move(action), duration_limit,
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

#endif // SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_BUILDER_GENERIC_ACTION_DECORATOR_H_
