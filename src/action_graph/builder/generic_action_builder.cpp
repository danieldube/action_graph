#include <action_graph/action_sequence.h>
#include <action_graph/builder/generic_action_builder.h>
#include <action_graph/parallel_actions.h>

namespace action_graph {
namespace builder {

using ::action_graph::Action;
using ActionObject = std::unique_ptr<Action>;
class ActionBuilder;
using BuilderFunction =
    std::function<ActionObject(const YAML::Node &, const ActionBuilder &)>;
using BuilderFunctions = std::map<std::string, BuilderFunction>;

ActionObject GenericActionBuilder::operator()(const YAML::Node &node) const {
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

void GenericActionBuilder::AddBuilderFunction(
    const std::string &action_type, BuilderFunction builder_function) {
  builder_functions_[action_type] = std::move(builder_function);
}

GenericActionBuilder CreateGenericActionBuilderWithDefaultActions() {

  GenericActionBuilder builder{};

  builder.AddBuilderFunction(
      "sequential_actions",
      [](const YAML::Node &node, const ActionBuilder &action_builder) {
        auto actions = BuildActions(node, action_builder);
        return std::make_unique<ActionSequence>(node["name"].as<std::string>(),
                                                std::move(actions));
      });
  builder.AddBuilderFunction(
      "parallel_actions",
      [](const YAML::Node &node, const ActionBuilder &action_builder) {
        auto actions = BuildActions(node, action_builder);
        return std::make_unique<ParallelActions>(node["name"].as<std::string>(),
                                                 std::move(actions));
      });
  return builder;
}

} // namespace builder
} // namespace action_graph
