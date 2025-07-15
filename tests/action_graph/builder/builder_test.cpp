#include <action_graph/builder/builder.h>
#include <action_graph/builder/generic_action_builder.h>
#include <gtest/gtest.h>
#include <native_configuration/map_node.h>
#include <native_configuration/scalar_node.h>
#include <native_configuration/sequence_node.h>
#include <string>

#include <action_graph/global_timer/global_timer.h>
#include <builder/callback_action.h>
#include <test_clock.h>

using namespace action_graph::native_configuration;
using action_graph::builder::ConfigurationNode;

const SequenceNode kSimpleGraph{
    MapNode{std::make_pair(
        "trigger",
        MapNode{
            std::make_pair("name", ScalarNode{"two_seconds"}),
            std::make_pair("period", ScalarNode{"2 seconds"}),
            std::make_pair(
                "action",
                MapNode{std::make_pair("name", ScalarNode{"action"}),
                        std::make_pair("type", ScalarNode{"callback_action"}),
                        std::make_pair("message",
                                       ScalarNode{"two seconds executed"})})})},
    MapNode{std::make_pair(
        "trigger",
        MapNode{
            std::make_pair("name", ScalarNode{"three_seconds"}),
            std::make_pair("period", ScalarNode{"3 seconds"}),
            std::make_pair(
                "action",
                MapNode{
                    std::make_pair("name", ScalarNode{"action"}),
                    std::make_pair("type", ScalarNode{"callback_action"}),
                    std::make_pair("message",
                                   ScalarNode{"three seconds executed"})})})}};

class BuildTriggerTest : public ::testing::Test {
protected:
  void SetUp() override {
    TestClock::reset();
    using action_graph::builder::ActionBuilder;
    auto &message_reference = this->message;
    action_builder.AddBuilderFunction(
        "callback_action", [&message_reference](const ConfigurationNode &node,
                                                const ActionBuilder &) {
          return CreateCallbackActionFromYaml(
              node, [&message_reference](const std::string &msg) {
                message_reference = msg;
              });
        });
  }

  void AdvanceTime(const TestClock::duration &duration) {
    TestClock::advance_time(duration);
    timer.WaitOneCycle();
    // This is a hack to workaround wrong signalling
    std::this_thread::sleep_for(std::chrono::milliseconds{10});
  }

  action_graph::GlobalTimer<TestClock> timer{};
  action_graph::builder::GenericActionBuilder action_builder{};
  std::string message;
};

TEST_F(BuildTriggerTest, BuildActionGraph_simple_graph) {
  using action_graph::builder::BuildActionGraph;

  auto action = BuildActionGraph(kSimpleGraph, action_builder, timer);

  EXPECT_EQ(message, "");

  AdvanceTime(std::chrono::seconds{1});
  EXPECT_EQ(message, "");

  AdvanceTime(std::chrono::seconds{1});
  EXPECT_EQ(message, "two seconds executed");

  AdvanceTime(std::chrono::seconds{1});
  EXPECT_EQ(message, "three seconds executed");
}

const MapNode kTriggerGraph{std::make_pair(
    "trigger",
    MapNode{std::make_pair("name", ScalarNode{"one_second"}),
            std::make_pair("period", ScalarNode{"1 seconds"}),
            std::make_pair(
                "action",
                MapNode{std::make_pair("name", ScalarNode{"action"}),
                        std::make_pair("type", ScalarNode{"callback_action"}),
                        std::make_pair("message",
                                       ScalarNode{"one second executed"})})})};

TEST_F(BuildTriggerTest, BuildTrigger_single_trigger) {
  using action_graph::builder::ActionBuilder;
  using action_graph::builder::BuildTrigger;
  using action_graph::builder::GenericActionBuilder;

  auto trigger = BuildTrigger(kTriggerGraph, action_builder, timer);

  EXPECT_EQ(message, "");

  AdvanceTime(std::chrono::seconds{1});
  EXPECT_EQ(message, "one second executed");
}
