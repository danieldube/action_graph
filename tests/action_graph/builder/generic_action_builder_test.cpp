#include <action_graph/builder/generic_action_builder.h>
#include <gtest/gtest.h>
#include <native_configuration/map_node.h>
#include <native_configuration/scalar_node.h>
#include <native_configuration/sequence_node.h>
#include <string>

#include "callback_action.h"

using namespace action_graph::native_configuration;

const MapNode kCallbackAction{std::make_pair(
    "action",
    MapNode{std::make_pair("name", ScalarNode{"action"}),
            std::make_pair("type", ScalarNode{"callback_action"}),
            std::make_pair("message", ScalarNode{"one second executed"})})};

using action_graph::builder::ConfigurationNode;

TEST(GenericActionBuilder, simple_action) {
  using action_graph::builder::ActionBuilder;
  using action_graph::builder::GenericActionBuilder;

  std::string message;
  GenericActionBuilder action_builder{};
  action_builder.AddBuilderFunction(
      "callback_action",
      [&message](const ConfigurationNode &node, const ActionBuilder &) {
        return CreateCallbackActionFromYaml(
            node, [&message](const std::string &msg) { message = msg; });
      });
  auto action = action_builder(kCallbackAction);
  action->Execute();
  EXPECT_EQ(message, "one second executed");
}

const MapNode kSequentialActions{std::make_pair(
    "action",
    MapNode{
        std::make_pair("name", ScalarNode{"action"}),
        std::make_pair("type", ScalarNode{"sequential_actions"}),
        std::make_pair(
            "actions",
            SequenceNode{
                MapNode{std::make_pair(
                    "action",
                    MapNode{
                        std::make_pair("name", ScalarNode{"action1"}),
                        std::make_pair("type", ScalarNode{"callback_action"}),
                        std::make_pair("message",
                                       ScalarNode{"action1 executed"})})},
                MapNode{std::make_pair(
                    "action",
                    MapNode{
                        std::make_pair("name", ScalarNode{"action2"}),
                        std::make_pair("type", ScalarNode{"callback_action"}),
                        std::make_pair("message",
                                       ScalarNode{"action2 executed"})})}})})};

TEST(GenericActionBuilder, sequential_actions) {
  using action_graph::builder::ActionBuilder;
  using action_graph::builder::CreateGenericActionBuilderWithDefaultActions;

  std::vector<std::string> messages;
  auto action_builder = CreateGenericActionBuilderWithDefaultActions();
  action_builder.AddBuilderFunction(
      "callback_action",
      [&messages](const ConfigurationNode &node, const ActionBuilder &) {
        return CreateCallbackActionFromYaml(
            node,
            [&messages](const std::string &msg) { messages.push_back(msg); });
      });
  auto action = action_builder(kSequentialActions);
  action->Execute();
  ASSERT_EQ(messages.size(), 2);
  EXPECT_EQ(messages.front(), "action1 executed");
  EXPECT_EQ(messages.back(), "action2 executed");
}

const MapNode kParallelActions{std::make_pair(
    "action",
    MapNode{
        std::make_pair("name", ScalarNode{"action"}),
        std::make_pair("type", ScalarNode{"parallel_actions"}),
        std::make_pair(
            "actions",
            SequenceNode{
                MapNode{std::make_pair(
                    "action",
                    MapNode{
                        std::make_pair("name", ScalarNode{"action1"}),
                        std::make_pair("type", ScalarNode{"callback_action"}),
                        std::make_pair("message",
                                       ScalarNode{"action1 executed"})})},
                MapNode{std::make_pair(
                    "action",
                    MapNode{
                        std::make_pair("name", ScalarNode{"action2"}),
                        std::make_pair("type", ScalarNode{"callback_action"}),
                        std::make_pair("message",
                                       ScalarNode{"action2 executed"})})}})})};

TEST(GenericActionBuilder, parallel_actions) {
  using action_graph::builder::ActionBuilder;
  using action_graph::builder::CreateGenericActionBuilderWithDefaultActions;

  std::mutex messages_mutex;
  std::vector<std::string> messages;
  auto action_builder = CreateGenericActionBuilderWithDefaultActions();
  action_builder.AddBuilderFunction(
      "callback_action",
      [&messages, &messages_mutex](const ConfigurationNode &node,
                                   const ActionBuilder &) {
        return CreateCallbackActionFromYaml(
            node, [&messages, &messages_mutex](const std::string &msg) {
              std::lock_guard guard(messages_mutex);
              messages.push_back(msg);
            });
      });

  auto action = action_builder(kParallelActions);

  action->Execute();

  auto has_message = std::find(messages.begin(), messages.end(),
                               "action1 executed") != messages.end();
  EXPECT_TRUE(has_message);
  has_message = std::find(messages.begin(), messages.end(),
                          "action2 executed") != messages.end();
  EXPECT_TRUE(has_message);
}
