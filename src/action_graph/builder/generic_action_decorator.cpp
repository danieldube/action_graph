#include <action_graph/builder/generic_action_decorator.h>

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

} // namespace builder
} // namespace action_graph
