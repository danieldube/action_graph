#include <action_graph/builder/builder.h>
#include <action_graph/builder/generic_action_builder.h>
#include <gtest/gtest.h>
#include <string>
#include <yaml-cpp/yaml.h>

#include "callback_action.h"

const std::string kCallbackAction = R"(
  action:
    name: action
    type: callback_action
    message: "one second executed"
)";

TEST(ActionBuilder, simple_action) {
  using action_graph::builder::ActionBuilder;
  using action_graph::builder::GenericActionBuilder;

  std::string message;
  GenericActionBuilder action_builder{};
  action_builder.AddBuilderFunction(
      "callback_action",
      [&message](const YAML::Node &node, const ActionBuilder &) {
        return CreateCallbackActionFromYaml(
            node, [&message](const std::string &msg) { message = msg; });
      });
  YAML::Node action_yml = YAML::Load(kCallbackAction);
  auto action = action_builder(action_yml);
  action->Execute();
  EXPECT_EQ(message, "one second executed");
}

const std::string kSimpleGraphYml = R"(
- trigger:
    name: one_second
    period: 1 seconds
    action:
      name: action
      type: callback_action
      message: "one second executed"
- trigger:
    name: two_seconds
    period: 2 seconds
    action:
      name: action
      type: callback_action
      message: "two seconds executed"
)";

TEST(ActionBuilder, simple_graph) {
  using action_graph::builder::ActionBuilder;
  using action_graph::builder::BuildActionGraph;
  using action_graph::builder::GenericActionBuilder;

  std::string message;

  GenericActionBuilder action_builder{};
  action_builder.AddBuilderFunction(
      "callback_action",
      [&message](const YAML::Node &node, const ActionBuilder &) {
        return CreateCallbackActionFromYaml(
            node, [&message](const std::string &msg) { message = msg; });
      });
  auto action = BuildActionGraph(kSimpleGraphYml, action_builder);
  ASSERT_EQ(action.size(), 2);
  action.front()->Execute();
  EXPECT_EQ(message, "one second executed");
  action.back()->Execute();
  EXPECT_EQ(message, "two seconds executed");
}
