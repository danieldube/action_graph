#ifndef ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_EXECUTION_OBSERVER_H_
#define ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_EXECUTION_OBSERVER_H_

#include <exception>

namespace action_graph {
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
} // namespace action_graph

#endif // ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_EXECUTION_OBSERVER_H_
