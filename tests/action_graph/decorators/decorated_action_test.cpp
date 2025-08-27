// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#include <gtest/gtest.h>

#include <action_graph/decorators/decorated_action.h>

using action_graph::Action;
using action_graph::decorators::DecoratedAction;

class PrintAction final : public Action {
public:
  PrintAction(std::string name, std::ostream &output_stream)
      : output_stream_(output_stream), Action(std::move(name)) {}

  void Execute() override { output_stream_ << name << " executed"; }

private:
  std::ostream &output_stream_;
};

class PrintActionDecorator final : public DecoratedAction {
public:
  explicit PrintActionDecorator(std::unique_ptr<Action> action,
                                std::ostream &output_stream)
      : DecoratedAction(std::move(action)), output_stream_(output_stream) {}

  void Execute() override {
    output_stream_ << "decorate(";
    GetAction().Execute();
    output_stream_ << ")";
  }

private:
  std::ostream &output_stream_;
};

TEST(DecoratedAction, decorate_action) {
  std::stringstream stream;
  auto action = std::make_unique<PrintAction>("test", stream);
  auto decorated_action =
      std::make_unique<PrintActionDecorator>(std::move(action), stream);
  decorated_action->Execute();
  EXPECT_EQ(stream.str(), "decorate(test executed)");
}
