#include <action_graph/action.h>
#include <action_graph/builder.h>
#include <gtest/gtest.h>
#include <string>
#include <yaml-cpp/yaml.h>

const std::string kSimpleGraphYml = R"(
trigger:
  name: ten_milliseconds
  period: 10ms
  action:
    name: action
    type: print_action
    message: "action executed"
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

TEST(BuildActionGraph, simple_graph_yml) {
  using action_graph::builder::ActionObject;
  using action_graph::builder::BuildActionGraph;

  std::string message;
  const action_graph::builder::ActionCreators actions{
      {std::string("print_action"), [&message](const YAML::Node &node) {
         return CreateCallbackActionFromYaml(
             node, [&message](const std::string &msg) { message = msg; });
       }}};
  auto graph = BuildActionGraph(kSimpleGraphYml, actions);
  ASSERT_EQ(graph.size(), 1);
  graph.front()->Execute();
  EXPECT_EQ(message, "action executed");
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
