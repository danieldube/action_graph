// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#ifndef ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_BUILDER_BUILDER_H_
#define ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_BUILDER_BUILDER_H_

#include <action_graph/action.h>
#include <action_graph/builder/configuration_node.h>
#include <action_graph/builder/parse_duration.h>
#include <action_graph/global_timer/global_timer.h>

#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <stdexcept>
#include <vector>

namespace action_graph {
namespace builder {
using ::action_graph::Action;
using ActionObject = std::unique_ptr<Action>;

class ActionBuilder {
public:
  virtual ~ActionBuilder() = default;
  virtual ActionObject operator()(const ConfigurationNode &node) const = 0;
};

class ConfigurationError : public std::runtime_error {
public:
  explicit ConfigurationError(
      const std::string &message,
      const action_graph::builder::ConfigurationNode &node)
      : std::runtime_error("Error parsing configuration: " + message + " :\n" +
                           node.AsString()) {}
};

class BuildError : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

template <typename Clock>
auto BuildActionGraph(const ConfigurationNode &configuration,
                      const ActionBuilder &action_builder,
                      action_graph::GlobalTimer<Clock> &global_timer)
    -> std::vector<ActionObject> {
  std::vector<ActionObject> created_actions;
  for (size_t entry_index = 0; entry_index < configuration.Size();
       ++entry_index) {
    const auto &entry = configuration.Get(entry_index);
    created_actions.push_back(
        BuildTrigger(entry, action_builder, global_timer));
  }
  return created_actions;
}

template <typename Clock>
ActionObject BuildTrigger(const ConfigurationNode &node,
                          const ActionBuilder &action_builder,
                          GlobalTimer<Clock> &global_timer) {
  if (!node.HasKey("trigger"))
    throw ConfigurationError("Only trigger nodes are allowed on top level.",
                             node);
  const auto &trigger = node.Get("trigger");
  auto trigger_name = trigger.Get("name").AsString();
  auto trigger_period_string = trigger.Get("period").AsString();
  auto trigger_period = ParseDuration(trigger_period_string);
  auto casted_trigger_period =
      std::chrono::duration_cast<typename Clock::duration>(trigger_period);

  auto action_pointer = action_builder(trigger);
  auto &action = *action_pointer;
  global_timer.SetTriggerTime(casted_trigger_period,
                              [&action]() { action.Execute(); });

  return action_pointer;
}
} // namespace builder
} // namespace action_graph

#endif // ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_BUILDER_BUILDER_H_
