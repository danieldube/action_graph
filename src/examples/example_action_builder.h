// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#ifndef ACTION_GRAPH_SRC_EXAMPLES_EXAMPLE_ACTION_BUILDER_H_
#define ACTION_GRAPH_SRC_EXAMPLES_EXAMPLE_ACTION_BUILDER_H_

#include <action_graph/builder/generic_action_builder.h>
#include <action_graph/builder/generic_action_decorator.h>

#include <memory>
#include <string>

#include "example_context.h"

class ExampleActionBuilder final : public action_graph::builder::ActionBuilder {
public:
  explicit ExampleActionBuilder(ExampleContext &context);

  action_graph::builder::ActionObject operator()(
      const action_graph::builder::ConfigurationNode &node) const override;

private:
  using ConfigurationNode = action_graph::builder::ConfigurationNode;
  using ActionObject = action_graph::builder::ActionObject;

  void RegisterActions();
  void RegisterDecorators();
  ActionObject BuildLogMessageAction(const ConfigurationNode &node) const;
  ActionObject BuildWaitAndLogAction(const ConfigurationNode &node) const;

  ExampleContext &context_;
  action_graph::builder::GenericActionBuilder action_builder_;
  action_graph::builder::GenericActionDecorator decorator_builder_;
};

#endif // ACTION_GRAPH_SRC_EXAMPLES_EXAMPLE_ACTION_BUILDER_H_
