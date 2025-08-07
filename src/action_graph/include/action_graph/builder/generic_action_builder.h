// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#ifndef ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_BUILDER_GENERIC_ACTION_BUILDER_H_
#define ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_BUILDER_GENERIC_ACTION_BUILDER_H_
#include <action_graph/builder/builder.h>

namespace action_graph {
namespace builder {

using BuilderFunction = std::function<ActionObject(const ConfigurationNode &,
                                                   const ActionBuilder &)>;
using BuilderFunctions = std::map<std::string, BuilderFunction>;

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
