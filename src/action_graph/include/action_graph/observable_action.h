#ifndef ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_OBSERVABLE_ACTION_H_
#define ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_OBSERVABLE_ACTION_H_

#include <action_graph/action.h>
#include <action_graph/execution_observer.h>
#include <memory>

namespace action_graph {
class ObservableAction : public Action {
public:
  ObservableAction(std::unique_ptr<Action> action,
                   std::unique_ptr<ExecutionObserver> observer)
      : Action(action->name), observer_(std::move(observer)),
        action_(std::move(action)) {}

  void Execute() override {
    observer_->OnStarted();

    try {
      action_->Execute();
    } catch (std::exception &exception) {
      observer_->OnFailed(exception);
      throw;
    }
    observer_->OnFinished();
  }

private:
  std::unique_ptr<ExecutionObserver> observer_;
  std::unique_ptr<Action> action_;
};
} // namespace action_graph

#endif // ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_OBSERVABLE_ACTION_H_
