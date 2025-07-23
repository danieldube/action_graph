#include <action_graph/builder/configuration_node.h>
#include <action_graph/builder/generic_action_decorator.h>
#include <action_graph/decorated_action.h>
#include <gtest/gtest.h>
#include <memory>
#include <native_configuration/map_node.h>
#include <native_configuration/scalar_node.h>
#include <native_configuration/sequence_node.h>
#include <sstream>

using action_graph::Action;

class PrintingAction : public Action {
public:
  explicit PrintingAction(std::ostream &stream)
      : stream_(stream), action_graph::Action("printing action") {}
  void Execute() override { stream_ << "TestAction"; }

private:
  std::ostream &stream_;
};

using action_graph::DecoratedAction;
using action_graph::builder::ActionObject;

class NameDecorator : public DecoratedAction {
public:
  NameDecorator(ActionObject action, std::string name, std::ostream &stream)
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
  ActionObject action = std::make_unique<PrintingAction>(output);

  using action_graph::builder::GenericActionDecorator;
  GenericActionDecorator decorator_builder;

  using action_graph::builder::ConfigurationNode;
  auto name_decorator = [&output](const ConfigurationNode &node,
                                  ActionObject action) {
    auto name = node.Get("name").AsString();
    return std::make_unique<NameDecorator>(std::move(action), name, output);
  };
  decorator_builder.AddDecoratorFunction("NameDecorator", name_decorator);

  action = decorator_builder(kCallbackAction, std::move(action));
  action->Execute();

  // Check output
  std::string expected = "second(first(TestAction))";
  EXPECT_EQ(output.str(), expected);
}
