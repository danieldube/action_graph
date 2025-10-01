// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.
#ifndef SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_DECORATORS_EXECUTION_OBSERVER_H_
#define SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_DECORATORS_EXECUTION_OBSERVER_H_

#include <exception>

namespace action_graph {
namespace decorators {
class ExecutionObserver {
public:
  virtual ~ExecutionObserver() = default;

  virtual void OnStarted() = 0;
  virtual void OnFinished() = 0;
  virtual void OnFailed(const std::exception &) = 0;
};

class NoOperationExecutionObserver : public ExecutionObserver {
public:
  void OnStarted() override {}
  void OnFinished() override {}
  void OnFailed(const std::exception &) override {}
};
} // namespace decorators
} // namespace action_graph

#endif // SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_DECORATORS_EXECUTION_OBSERVER_H_
