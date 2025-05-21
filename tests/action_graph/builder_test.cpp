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

  std::vector<std::string> messages;
  BuilderFunctions actions{
      {"callback_action",
       [&messages](const YAML::Node &node, const ActionBuilder &) {
         return CreateCallbackActionFromYaml(
             node,
             [&messages](const std::string &msg) { messages.push_back(msg); });
       }}};
  ActionBuilder action_builder(actions);
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
  std::mutex messages_mutex;
  std::vector<std::string> messages;
  BuilderFunctions actions{
      {"callback_action", [&messages, &messages_mutex](const YAML::Node &node,
                                                       const ActionBuilder &) {
         return CreateCallbackActionFromYaml(
             node, [&messages, &messages_mutex](const std::string &msg) {
               std::lock_guard message_lock(messages_mutex);
               messages.push_back(msg);
             });
       }}};
  ActionBuilder action_builder(actions);
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
