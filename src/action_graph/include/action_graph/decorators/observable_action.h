// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#ifndef ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_DECORATORS_OBSERVABLE_ACTION_H_
#define ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_DECORATORS_OBSERVABLE_ACTION_H_

#include <action_graph/decorators/decorated_action.h>
#include <action_graph/decorators/execution_observer.h>
#include <memory>

namespace action_graph {
namespace decorators {
class ObservableAction : public DecoratedAction {
public:
  using Action = action_graph::Action;
  ObservableAction(std::unique_ptr<Action> action,
                   std::unique_ptr<ExecutionObserver> observer)
      : DecoratedAction(std::move(action)), observer_(std::move(observer)) {}

  void Execute() override {
    observer_->OnStarted();

    try {
      GetAction().Execute();
    } catch (std::exception &exception) {
      observer_->OnFailed(exception);
      throw;
    }
    observer_->OnFinished();
  }

private:
  std::unique_ptr<ExecutionObserver> observer_;
};
} // namespace decorators
} // namespace action_graph

#endif // ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_DECORATORS_OBSERVABLE_ACTION_H_
