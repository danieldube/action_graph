// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#ifndef ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_BUILDER_GENERIC_ACTION_DECORATOR_H_
#define ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_BUILDER_GENERIC_ACTION_DECORATOR_H_

#include "builder.h"
#include <action_graph/action.h>
#include <action_graph/builder/configuration_node.h>
#include <functional>
#include <map>
#include <string>

namespace action_graph {
namespace builder {

using DecorateFunction =
    std::function<ActionObject(const ConfigurationNode &, ActionObject action)>;
using DecorateFunctions = std::map<std::string, DecorateFunction>;

class GenericActionDecorator {
public:
  GenericActionDecorator() = default;
  ActionObject operator()(const ConfigurationNode &node,
                          ActionObject action) const;
  void AddDecoratorFunction(const std::string &action_type,
                            DecorateFunction decorate_function);

private:
  DecorateFunctions decorate_functions_;
};
} // namespace builder
} // namespace action_graph

#endif // ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_BUILDER_GENERIC_ACTION_DECORATOR_H_
