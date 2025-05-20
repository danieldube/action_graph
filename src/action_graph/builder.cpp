#include <action_graph/action.h>
#include <action_graph/builder.h>

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

ActionBuilder::ActionBuilder(
    action_graph::builder::BuilderFunctions builder_functions)
    : builder_functions_(std::move(builder_functions)) {}

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

std::chrono::duration<double> ParseDuration(const std::string &duration_str) {
  if (duration_str.find("nanoseconds") != std::string::npos) {
    return std::chrono::nanoseconds(std::stoi(duration_str));
  } else if (duration_str.find("microseconds") != std::string::npos) {
    return std::chrono::microseconds(std::stoi(duration_str));
  } else if (duration_str.find("milliseconds") != std::string::npos) {
    return std::chrono::milliseconds(std::stoi(duration_str));
  } else if (duration_str.find("seconds") != std::string::npos) {
    return std::chrono::seconds(std::stoi(duration_str));
  } else {
    throw std::invalid_argument("Invalid duration format");
  }
}

} // namespace builder
} // namespace action_graph
