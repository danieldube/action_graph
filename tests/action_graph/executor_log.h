// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#ifndef ACTION_GRAPH_TESTS_ACTION_GRAPH_EXECUTOR_LOG_H_
#define ACTION_GRAPH_TESTS_ACTION_GRAPH_EXECUTOR_LOG_H_
#include <action_graph/action.h>
#include <gtest/gtest.h>
#include <thread>

class ExecutorLog {
public:
  void Log(const std::string &message) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    log_.push_back(message);
  }

  std::vector<std::string> GetLog() {
    std::lock_guard<std::mutex> lock(log_mutex_);
    return log_;
  }

private:
  std::vector<std::string> log_{};
  std::mutex log_mutex_;
};

class LoggingAction final : public action_graph::Action {
public:
  LoggingAction(std::string name, ExecutorLog &executor_log)
      : Action(std::move(name)), log_{executor_log} {}

  using action_graph::Action::Action;

  void Execute() override {
    log_.Log("start: " + name);
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    log_.Log("stop: " + name);
  }

private:
  ExecutorLog &log_;
};

#endif // ACTION_GRAPH_TESTS_ACTION_GRAPH_EXECUTOR_LOG_H_
