#include <action_graph/single_action.h>
#include <gtest/gtest.h>

using action_graph::Action;

class TestAction final : public Action {
public:
  TestAction(std::string name, bool &mark_when_execute)
      : Action(std::move(name)), mark_when_execute_{mark_when_execute} {}
  using action_graph::Action::Action;
  ~TestAction() override {
    std::cout << "TestAction destructor called for: " << name << std::endl;
  }

  void Execute() override { mark_when_execute_ = true; }

private:
  bool &mark_when_execute_;
};

TEST(simple, test) {
  bool was_executed = false;
  TestAction action("test_action", was_executed);
  action.Execute();
  EXPECT_TRUE(was_executed);
}
