#include <action_graph/builder/configuration_node.h>
#include <action_graph/builder/generic_action_decorator.h>
#include <action_graph/decorated_action.h>
#include <gtest/gtest.h>
#include <memory>
#include <native_configuration/map_node.h>
#include <native_configuration/scalar_node.h>
#include <native_configuration/sequence_node.h>
#include <sstream>

class PrintingAction : public action_graph::Action {
public:
  explicit PrintingAction(std::ostream &stream)
      : stream_(stream), action_graph::Action("dummy") {}
  void Execute() override { stream_ << "TestAction"; }

private:
  std::ostream &stream_;
};

// Decorator: adds name before action execution
class NameDecorator : public action_graph::DecoratedAction {
public:
  NameDecorator(std::unique_ptr<Action> action, std::string name,
                std::ostream &stream)
      : action_graph::DecoratedAction(std::move(action)),
        name_(std::move(name)), stream_(stream) {}

  void Execute() override {
    stream_ << name_ << "(";
    GetAction().Execute();
    stream_ << ")";
  }

private:
  std::string name_;
  std::ostream &stream_;
};

using namespace action_graph::native_configuration;

const MapNode kCallbackAction{std::make_pair(
    "decorate",
    SequenceNode{MapNode{std::make_pair("type", ScalarNode{"NameDecorator"}),
                         std::make_pair("name", ScalarNode{"first"})},
                 MapNode{std::make_pair("type", ScalarNode{"NameDecorator"}),
                         std::make_pair("name", ScalarNode{"second"})}})};

TEST(GenericActionDecoratorTest, DecorateActionWithNameDecorator) {
  std::stringstream output;
  auto action = std::make_unique<PrintingAction>(output);

  action_graph::builder::GenericActionDecorator decorator_builder;
  auto namedecorator =
      [&output](const action_graph::builder::ConfigurationNode &node,
                action_graph::builder::ActionObject action) {
        auto name = node.Get("name").AsString();
        return std::make_unique<NameDecorator>(std::move(action), name, output);
      };
  decorator_builder.AddDecoratorFunction("NameDecorator", namedecorator);

  auto new_action = decorator_builder(kCallbackAction, std::move(action));
  new_action->Execute();

  // Check output
  std::string expected = "second(first(TestAction))";
  EXPECT_EQ(output.str(), expected);
}
