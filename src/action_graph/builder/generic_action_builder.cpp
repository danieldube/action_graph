#include <action_graph/action_sequence.h>
#include <action_graph/builder/configuration_node.h>
#include <action_graph/builder/generic_action_builder.h>
#include <action_graph/parallel_actions.h>

namespace action_graph {
namespace builder {

using ::action_graph::Action;

std::vector<ActionObject> BuildActions(const ConfigurationNode &node,
                                       const ActionBuilder &action_builder) {
  std::vector<ActionObject> actions;
  if (!node.HasKey("actions")) {
    throw ConfigurationError("Actions are not defined.", node);
  }
  const auto &actions_node = node.Get("actions");
  for (size_t entry_index = 0; entry_index < actions_node.Size();
       ++entry_index) {
    const auto &action = actions_node.Get(entry_index);
    actions.push_back(action_builder(action));
  }
  return actions;
}

ActionObject
GenericActionBuilder::operator()(const ConfigurationNode &node) const {
  if (!node.HasKey("action")) {
    throw ConfigurationError(
        "The ActionBuilder can just be called on action nodes.", node);
  }
  const auto &action = node.Get("action");
  if (!action.HasKey("type")) {
    throw ConfigurationError("Type of the action is not defined.", node);
  }
  auto action_type = action.Get("type").AsString();

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
      [](const ConfigurationNode &node, const ActionBuilder &action_builder) {
        auto actions = BuildActions(node, action_builder);
        return std::make_unique<ActionSequence>(node.Get("name").AsString(),
                                                std::move(actions));
      });
  builder.AddBuilderFunction(
      "parallel_actions",
      [](const ConfigurationNode &node, const ActionBuilder &action_builder) {
        auto actions = BuildActions(node, action_builder);
        return std::make_unique<ParallelActions>(node.Get("name").AsString(),
                                                 std::move(actions));
      });
  return builder;
}

} // namespace builder
} // namespace action_graph
