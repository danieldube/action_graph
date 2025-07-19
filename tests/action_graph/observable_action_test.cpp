#include <gtest/gtest.h>

#include <action_graph/observable_action.h>
#include <thread>

using action_graph::Action;

class HundredMillisecondsAction final : public Action {
public:
  HundredMillisecondsAction() : Action("100ms Action") {}

  void Execute() override {
    constexpr auto hundred_milliseconds = std::chrono::milliseconds(100);
    std::this_thread::sleep_for(hundred_milliseconds);
  }

private:
};

using action_graph::ExecutionObserver;
class TestExecutionObserver : public ExecutionObserver {
public:
  explicit TestExecutionObserver(std::ostream &log) : log_(log) {}

  void OnStarted() override { log_ << "Execution started."; }

  void OnFinished() override { log_ << "Execution finished."; }

  void OnFailed(const std::exception &exception) override {
    const std::string error_message =
        "Execution skipped: " + std::string(exception.what());
    log_ << "Execution failed: " << exception.what();
  }

private:
  std::ostream &log_;
};

TEST(ObservableAction, execute) {
  using action_graph::ObservableAction;
  std::stringstream log;
  auto action = std::make_unique<HundredMillisecondsAction>();
  auto observer = std::make_unique<TestExecutionObserver>(log);
  ObservableAction observable_action(std::move(action), std::move(observer));
  observable_action.Execute();

  EXPECT_EQ(log.str(), "Execution started."
                       "Execution finished.");
}

class ThrowingAction final : public Action {
public:
  ThrowingAction() : Action("Throwing Action") {}

  void Execute() override {
    throw std::runtime_error("This Action always throws.");
  }
};

TEST(ObservableAction, execute_with_exception) {
  using action_graph::ObservableAction;
  std::stringstream log;
  auto action = std::make_unique<ThrowingAction>();
  auto observer = std::make_unique<TestExecutionObserver>(log);
  ObservableAction observable_action(std::move(action), std::move(observer));

  EXPECT_THROW(observable_action.Execute(), std::runtime_error);

  EXPECT_EQ(log.str(), "Execution started."
                       "Execution failed: This Action always throws.");
}
