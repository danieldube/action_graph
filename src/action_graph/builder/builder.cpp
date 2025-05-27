#include "action_graph/builder/builder.h"
#include "action_graph/action.h"
#include "action_graph/action_sequence.h"
#include "action_graph/parallel_actions.h"
#include <action_graph/builder/parse_duration.h>

namespace action_graph {
namespace builder {

ActionObject BuildTrigger(const YAML::Node &node,
                          const ActionBuilder &action_builder) {
  const auto &trigger = node["trigger"];
  if (!trigger) {
    throw YamlParsingError("Only trigger nodes are allowed on top level.",
                           node);
  }
  auto trigger_name = trigger["name"].as<std::string>();
  auto trigger_period_string = trigger["period"].as<std::string>();
  auto trigger_period = ParseDuration(trigger_period_string);
  return action_builder(trigger);
}

std::vector<ActionObject> BuildActions(const YAML::Node &node,
                                       const ActionBuilder &action_builder) {
  std::vector<ActionObject> actions;
  const auto &actions_node = node["actions"];
  if (!actions_node) {
    throw YamlParsingError("Actions are not defined.", node);
  }
  std::transform(actions_node.begin(), actions_node.end(),
                 std::back_inserter(actions),
                 [&action_builder](const YAML::Node &action) {
                   return action_builder(action);
                 });
  return actions;
}

ActionBuilder::ActionBuilder(
    action_graph::builder::BuilderFunctions builder_functions)
    : builder_functions_(std::move(builder_functions)) {
  builder_functions_.emplace(
      "sequential_actions",
      [](const YAML::Node &node, const ActionBuilder &action_builder) {
        auto actions = BuildActions(node, action_builder);
        return std::make_unique<ActionSequence>(node["name"].as<std::string>(),
                                                std::move(actions));
      });
  builder_functions_.emplace(
      "parallel_actions",
      [](const YAML::Node &node, const ActionBuilder &action_builder) {
        auto actions = BuildActions(node, action_builder);
        return std::make_unique<ParallelActions>(node["name"].as<std::string>(),
                                                 std::move(actions));
      });
}

ActionObject ActionBuilder::operator()(const YAML::Node &node) const {
  auto action = node["action"];
  if (!action) {
    throw YamlParsingError(
        "The ActionBuilder can just be called on action nodes.", node);
  }
  auto action_type = action["type"].as<std::string>();
  if (action_type.empty()) {
    throw YamlParsingError("Type of the action is not defined.", action);
  }
  auto builder = builder_functions_.find(action_type);
  if (builder == builder_functions_.end()) {
    throw BuildError("No builder defined for " + action_type + ".");
  }
  const auto &builder_function = builder->second;
  return builder_function(action, *this);
}

std::vector<ActionObject>
BuildActionGraph(const std::string &yaml_string,
                 const BuilderFunctions &builder_functions) {
  std::vector<ActionObject> created_actions;
  YAML::Node config = YAML::Load(yaml_string);
  ActionBuilder action_builder(builder_functions);
  std::transform(config.begin(), config.end(),
                 std::back_inserter(created_actions),
                 [&action_builder](const YAML::Node &entry) {
                   return BuildTrigger(entry, action_builder);
                 });
  return created_actions;
}

} // namespace builder
} // namespace action_graph
