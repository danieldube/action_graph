#include <action_graph/action.h>
#include <action_graph/action_sequence.h>
#include <action_graph/builder.h>
#include <gtest/gtest.h>
#include <string>
#include <yaml-cpp/yaml.h>

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

const std::string kCallbackAction = R"(
  action:
    name: action
    type: callback_action
    message: "one second executed"
)";

class CallbackAction final : public action_graph::Action {
public:
  CallbackAction(std::string name, std::string message,
                 std::function<void(const std::string &)> callback)
      : action_graph::Action(std::move(name)), message_(std::move(message)),
        callback_(std::move(callback)) {}

  void Execute() override { callback_(message_); }

private:
  std::string message_;
  std::function<void(const std::string &)> callback_;
};

std::unique_ptr<action_graph::Action> CreateCallbackActionFromYaml(
    const YAML::Node &node,
    std::function<void(const std::string &message)> callback) {
  return std::make_unique<CallbackAction>(node["name"].as<std::string>(),
                                          node["message"].as<std::string>(),
                                          std::move(callback));
}

TEST(ActionBuilder, simple_action) {
  using action_graph::builder::ActionBuilder;
  using action_graph::builder::ActionObject;
  using action_graph::builder::BuildActionGraph;
  using action_graph::builder::BuilderFunctions;

  std::string message;
  BuilderFunctions actions{
      {"callback_action",
       [&message](const YAML::Node &node, const ActionBuilder &) {
         return CreateCallbackActionFromYaml(
             node, [&message](const std::string &msg) { message = msg; });
       }}};
  ActionBuilder action_builder(actions);
  YAML::Node action_yml = YAML::Load(kCallbackAction);
  auto action = action_builder(action_yml);
  action->Execute();
  EXPECT_EQ(message, "one second executed");
}

TEST(ActionBuilder, simple_graph) {
  using action_graph::builder::ActionBuilder;
  using action_graph::builder::ActionObject;
  using action_graph::builder::BuildActionGraph;
  using action_graph::builder::BuilderFunctions;

  std::string message;
  BuilderFunctions actions{
      {"callback_action",
       [&message](const YAML::Node &node, const ActionBuilder &) {
         return CreateCallbackActionFromYaml(
             node, [&message](const std::string &msg) { message = msg; });
       }}};
  auto action = BuildActionGraph(kSimpleGraphYml, actions);
  ASSERT_EQ(action.size(), 2);
  action.front()->Execute();
  EXPECT_EQ(message, "one second executed");
  action.back()->Execute();
  EXPECT_EQ(message, "two seconds executed");
}

TEST(ParseDuration, seconds) {
  EXPECT_EQ(action_graph::builder::ParseDuration("10 seconds"),
            std::chrono::seconds(10));
}

TEST(ParseDuration, milliseconds) {
  EXPECT_EQ(action_graph::builder::ParseDuration("10 milliseconds"),
            std::chrono::milliseconds(10));
}

TEST(ParseDuration, microseconds) {
  EXPECT_EQ(action_graph::builder::ParseDuration("10 microseconds"),
            std::chrono::microseconds(10));
}

TEST(ParseDuration, nanoseconds) {
  EXPECT_EQ(action_graph::builder::ParseDuration("10 nanoseconds"),
            std::chrono::nanoseconds(10));
}
/*
const std::string kSequentialGraph = R"(
- trigger:
    name: one_second
    period: 1 seconds
    action:
      name: action
      type: sequential_action
      actions:
        - name: action1
          type: callback_action
          message: "action1 executed"
        - name: action2
          type: callback_action
          message: "action2 executed"
)";

std::unique_ptr<action_graph::Action> CreateSequentialActionFromYaml(
    const YAML::Node &node,
    std::function<void(const std::string &message)> callback) {
  for (const auto &action : node["actions"]) {
    auto action_type = action["type"].as<std::string>();
    if (action_type == "callback_action") {
      callback(action["message"].as<std::string>());
    }
  }
  return
std::make_unique<action_graph::ActionSequence>(node["name"].as<std::string>(),
                                          node["message"].as<std::string>(),
                                          std::move(callback));
}


TEST(BuildActionGraph, sequential_graph) {
  using action_graph::builder::ActionObject;
  using action_graph::builder::BuildActionGraph;

  std::string message;
  const action_graph::builder::BuilderFunctions actions{
      {"sequential_action", [&message](const YAML::Node &node) {
         return CreateSequentialActionFromYaml(
             node, [&message](const std::string &msg) { message = msg; });
       }},
      {"callback_action", [&message](const YAML::Node &node) {
         return CreateCallbackActionFromYaml(
             node, [&message](const std::string &msg) { message = msg; });
       }},
  };
  auto graph = BuildActionGraph(kSequentialGraph, actions);
  ASSERT_EQ(graph.size(), 2);
  graph.front()->Execute();
  EXPECT_EQ(message, "one second executed");
}
*/
