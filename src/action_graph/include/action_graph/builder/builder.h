#ifndef ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_BUILDER_BUILDER_H_
#define ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_BUILDER_BUILDER_H_

#include "action_graph/global_timer/global_timer.h"
#include <action_graph/action.h>
#include <action_graph/builder/parse_duration.h>

#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <yaml-cpp/yaml.h>

namespace action_graph {
namespace builder {
using ::action_graph::Action;
using ActionObject = std::unique_ptr<Action>;
class ActionBuilder;
using BuilderFunction =
    std::function<ActionObject(const YAML::Node &, const ActionBuilder &)>;
using BuilderFunctions = std::map<std::string, BuilderFunction>;

class ActionBuilder {
public:
  virtual ActionObject operator()(const YAML::Node &node) const = 0;
};

class YamlParsingError : public std::runtime_error {
public:
  explicit YamlParsingError(const std::string &message, const YAML::Node &node)
      : std::runtime_error("Error parsing yaml node: " + message + " :\n" +
                           YAML::Dump(node)) {}
};

class BuildError : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

template <typename Clock>
auto BuildActionGraph(const std::string &yaml_string,
                      const ActionBuilder &action_builder,
                      action_graph::GlobalTimer<Clock> &global_timer)
    -> std::vector<ActionObject> {
  std::vector<ActionObject> created_actions;
  YAML::Node config = YAML::Load(yaml_string);
  std::transform(config.begin(), config.end(),
                 std::back_inserter(created_actions),
                 [&action_builder, &global_timer](const YAML::Node &entry) {
                   return BuildTrigger(entry, action_builder, global_timer);
                 });
  return created_actions;
}

template <typename Clock>
ActionObject BuildTrigger(const YAML::Node &node,
                          const ActionBuilder &action_builder,
                          GlobalTimer<Clock> &global_timer) {
  const auto &trigger = node["trigger"];
  if (!trigger) {
    throw YamlParsingError("Only trigger nodes are allowed on top level.",
                           node);
  }
  auto trigger_name = trigger["name"].as<std::string>();
  auto trigger_period_string = trigger["period"].as<std::string>();
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
