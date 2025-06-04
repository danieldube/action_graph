#include <action_graph/builder/builder.h>
#include <action_graph/builder/generic_action_builder.h>
#include <gtest/gtest.h>
#include <string>
#include <yaml-cpp/yaml.h>

#include <action_graph/global_timer.h>
#include <builder/callback_action.h>
#include <test_clock.h>

const std::string kSimpleGraphYml = R"(
- trigger:
    name: two_seconds
    period: 2 seconds
    action:
      name: action
      type: callback_action
      message: "two seconds executed"
- trigger:
    name: three_seconds
    period: 3 seconds
    action:
      name: action
      type: callback_action
      message: "three seconds executed"
)";

class BuildTriggerTest : public ::testing::Test {
protected:
  void SetUp() override {
    TestClock::reset();
    using action_graph::builder::ActionBuilder;
    auto &message_reference = this->message;
    action_builder.AddBuilderFunction(
        "callback_action",
        [&message_reference](const YAML::Node &node, const ActionBuilder &) {
          return CreateCallbackActionFromYaml(
              node, [&message_reference](const std::string &msg) {
                message_reference = msg;
              });
        });
  }

  void TearDown() override { TestClock::reset(); }

  static void AdvanceTime(const TestClock::duration &duration) {
    TestClock::advance_time(duration);
    std::this_thread::sleep_for(std::chrono::milliseconds{5});
  }

  action_graph::GlobalTimer<TestClock> timer{};
  action_graph::builder::GenericActionBuilder action_builder{};
  std::string message;
};

TEST_F(BuildTriggerTest, BuildActionGraph_simple_graph) {
  using action_graph::builder::BuildActionGraph;

  auto action = BuildActionGraph(kSimpleGraphYml, action_builder, timer);

  EXPECT_EQ(message, "");

  AdvanceTime(std::chrono::seconds{1});
  EXPECT_EQ(message, "");

  AdvanceTime(std::chrono::seconds{1});
  EXPECT_EQ(message, "two seconds executed");

  AdvanceTime(std::chrono::seconds{1});
  EXPECT_EQ(message, "three seconds executed");
}

const std::string kTriggerYaml = R"(
- trigger:
    name: one_second
    period: 1 seconds
    action:
      name: action
      type: callback_action
      message: "one second executed"
)";

TEST_F(BuildTriggerTest, BuildTrigger_single_trigger) {
  using action_graph::builder::ActionBuilder;
  using action_graph::builder::BuildTrigger;
  using action_graph::builder::GenericActionBuilder;

  YAML::Node node = YAML::Load(kTriggerYaml);
  auto trigger = BuildTrigger(node[0], action_builder, timer);

  EXPECT_EQ(message, "");

  AdvanceTime(std::chrono::seconds{1});
  EXPECT_EQ(message, "one second executed");
}
