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

std::vector<ActionObject>
BuildActionGraph(const std::string &yaml_string,
                 const ActionBuilder &action_builder) {
  std::vector<ActionObject> created_actions;
  YAML::Node config = YAML::Load(yaml_string);
  std::transform(config.begin(), config.end(),
                 std::back_inserter(created_actions),
                 [&action_builder](const YAML::Node &entry) {
                   return BuildTrigger(entry, action_builder);
                 });
  return created_actions;
}

} // namespace builder
} // namespace action_graph
