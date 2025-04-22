#ifndef SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_ACTION_SEQUENCE_H_
#define SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_ACTION_SEQUENCE_H_

#include <action_graph/action.h>
#include <atomic>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

namespace action_graph {

class ExecutionError : public std::logic_error {
public:
  using std::logic_error::logic_error;
};

struct SequenceEntry {
  explicit SequenceEntry(std::unique_ptr<Action> action_to_call)
      : action(std::move(action_to_call)) {}
  std::unique_ptr<Action> action;
  std::atomic<bool> is_executed{false};
};

class ActionSequence final : public Action {
public:
  using Action::Action;

  template <class... Actions>
  explicit ActionSequence(std::string name, std::unique_ptr<Actions>... actions)
      : Action(std::move(name)) {
    (sequence_.emplace_back(std::move(actions)), ...);
  }

  explicit ActionSequence(std::string name,
                          std::vector<std::unique_ptr<Action>> actions)
      : Action(std::move(name)), sequence_(std::move(actions)) {}

  void Execute() override {
    for (auto &action : sequence_) {
      action->Execute();
    }
  }

private:
  std::vector<std::unique_ptr<Action>> sequence_;
};
} // namespace action_graph

#endif
