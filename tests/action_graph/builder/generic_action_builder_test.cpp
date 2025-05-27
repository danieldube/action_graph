#include "action_graph/action_sequence.h"
#include "action_graph/builder/generic_action_builder.h"
#include "yaml-cpp/yaml.h"
#include <gtest/gtest.h>
#include <string>

#include "callback_action.h"

const std::string kSequentialActions = R"(
  action:
    name: action
    type: sequential_actions
    actions:
        - action:
            name: action1
            type: callback_action
            message: "action1 executed"
        - action:
            name: action2
            type: callback_action
            message: "action2 executed"
)";

TEST(ActionBuilder, sequential_actions) {
  using action_graph::builder::ActionBuilder;
  using action_graph::builder::ActionObject;
  using action_graph::builder::BuildActionGraph;
  using action_graph::builder::BuilderFunctions;
  using action_graph::builder::CreateGenericActionBuilderWithDefaultActions;
  using action_graph::builder::GenericActionBuilder;

  std::vector<std::string> messages;
  BuilderFunctions actions{{}};
  auto action_builder = CreateGenericActionBuilderWithDefaultActions();
  action_builder.AddBuilderFunction(
      "callback_action",
      [&messages](const YAML::Node &node, const ActionBuilder &) {
        return CreateCallbackActionFromYaml(
            node,
            [&messages](const std::string &msg) { messages.push_back(msg); });
      });
  YAML::Node action_yml = YAML::Load(kSequentialActions);
  auto action = action_builder(action_yml);
  action->Execute();
  ASSERT_EQ(messages.size(), 2);
  EXPECT_EQ(messages.front(), "action1 executed");
  EXPECT_EQ(messages.back(), "action2 executed");
}

const std::string kParallelActions = R"(
  action:
    name: action
    type: parallel_actions
    actions:
        - action:
            name: action1
            type: callback_action
            message: "action1 executed"
        - action:
            name: action2
            type: callback_action
            message: "action2 executed"
)";

TEST(ActionBuilder, parallel_actions) {
  using action_graph::builder::ActionBuilder;
  using action_graph::builder::ActionObject;
  using action_graph::builder::BuildActionGraph;
  using action_graph::builder::BuilderFunctions;
  using action_graph::builder::CreateGenericActionBuilderWithDefaultActions;
  using action_graph::builder::GenericActionBuilder;

  std::mutex messages_mutex;
  std::vector<std::string> messages;
  auto action_builder = CreateGenericActionBuilderWithDefaultActions();
  action_builder.AddBuilderFunction(
      "callback_action", [&messages, &messages_mutex](const YAML::Node &node,
                                                      const ActionBuilder &) {
        return CreateCallbackActionFromYaml(
            node, [&messages, &messages_mutex](const std::string &msg) {
              std::lock_guard guard(messages_mutex);
              messages.push_back(msg);
            });
      });

  YAML::Node action_yml = YAML::Load(kParallelActions);
  auto action = action_builder(action_yml);

  action->Execute();

  auto has_message = std::find(messages.begin(), messages.end(),
                               "action1 executed") != messages.end();
  EXPECT_TRUE(has_message);
  has_message = std::find(messages.begin(), messages.end(),
                          "action2 executed") != messages.end();
  EXPECT_TRUE(has_message);
}
