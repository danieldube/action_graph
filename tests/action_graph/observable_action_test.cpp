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

class Log {
public:
  void Info(const std::string &message) {
    log_stream_ << "[INFO] " << message << std::endl;
  }

  void Error(const std::string &message) {
    log_stream_ << "[ERROR] " << message << std::endl;
  }

  std::string GetLog() const { return log_stream_.str(); }

private:
  std::stringstream log_stream_;
};

using action_graph::ExecutionObserver;
class TestExecutionObserver : public ExecutionObserver {
public:
  explicit TestExecutionObserver(Log &log) : log_(log) {}

  void OnStarted() override { log_.Info("Execution started."); }

  void OnFinished() override { log_.Info("Execution finished."); }

  void OnFailed(const std::exception &exception) override {
    const std::string error_message =
        "Execution skipped: " + std::string(exception.what());
    log_.Error(error_message);
  }

private:
  Log &log_;
};

TEST(ObservableAction, execute) {
  using action_graph::ObservableAction;
  Log log;
  auto action = std::make_unique<HundredMillisecondsAction>();
  auto observer = std::make_unique<TestExecutionObserver>(log);
  ObservableAction observable_action(std::move(action), std::move(observer));
  observable_action.Execute();

  EXPECT_EQ(log.GetLog(), "[INFO] Execution started.\n"
                          "[INFO] Execution finished.\n");
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
  Log log;
  auto action = std::make_unique<ThrowingAction>();
  auto observer = std::make_unique<TestExecutionObserver>(log);
  ObservableAction observable_action(std::move(action), std::move(observer));

  EXPECT_THROW(observable_action.Execute(), std::runtime_error);

  EXPECT_EQ(log.GetLog(),
            "[INFO] Execution started.\n"
            "[ERROR] Execution skipped: This Action always throws.\n");
}
