#ifndef ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_BUILDER_GENERIC_ACTION_BUILDER_H_
#define ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_BUILDER_GENERIC_ACTION_BUILDER_H_
#include <action_graph/builder/builder.h>

namespace action_graph {
namespace builder {
class GenericActionBuilder final : public ActionBuilder {
public:
  GenericActionBuilder() = default;
  ActionObject operator()(const ConfigurationNode &node) const override;
  void AddBuilderFunction(const std::string &action_type,
                          BuilderFunction builder_function);

private:
  BuilderFunctions builder_functions_;
};

std::vector<ActionObject> BuildActions(const ConfigurationNode &node,
                                       const ActionBuilder &action_builder);

GenericActionBuilder CreateGenericActionBuilderWithDefaultActions();
} // namespace builder
} // namespace action_graph
#endif // ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_BUILDER_GENERIC_ACTION_BUILDER_H_
