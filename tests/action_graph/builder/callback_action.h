// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.
#ifndef TESTS_ACTION_GRAPH_BUILDER_CALLBACK_ACTION_H_
#define TESTS_ACTION_GRAPH_BUILDER_CALLBACK_ACTION_H_

#include <action_graph/action.h>
#include <action_graph/builder/configuration_node.h>
#include <functional>
#include <memory>
#include <string>

class CallbackAction final : public action_graph::Action {
public:
  CallbackAction(std::string name, std::string message,
                 std::function<void(const std::string &)> callback);

  void Execute() override;

private:
  std::string message_;
  std::function<void(const std::string &)> callback_;
};

std::unique_ptr<action_graph::Action> CreateCallbackActionFromYaml(
    const action_graph::builder::ConfigurationNode &node,
    std::function<void(const std::string &message)> callback);

#endif // TESTS_ACTION_GRAPH_BUILDER_CALLBACK_ACTION_H_
