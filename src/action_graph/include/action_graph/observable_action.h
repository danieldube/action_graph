#ifndef ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_OBSERVABLE_ACTION_H_
#define ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_OBSERVABLE_ACTION_H_

#include <action_graph/decorated_action.h>
#include <action_graph/execution_observer.h>
#include <memory>

namespace action_graph {
class ObservableAction : public DecoratedAction {
public:
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
} // namespace action_graph

#endif // ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_OBSERVABLE_ACTION_H_
