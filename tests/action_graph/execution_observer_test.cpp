// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#include <gtest/gtest.h>

#include <action_graph/execution_observer.h>

using action_graph::ExecutionObserver;

TEST(ExecutionObserver, default_implementation) {
  class TestObserver : public ExecutionObserver {
  public:
    void OnStarted() override {}
    void OnFinished() override {}
    void OnFailed(const std::exception &exception) override {}
  };

  TestObserver observer;
  EXPECT_NO_THROW(observer.OnStarted());
  EXPECT_NO_THROW(observer.OnFinished());
  EXPECT_NO_THROW(observer.OnFailed(std::runtime_error("Test error")));
}
