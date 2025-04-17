#ifndef SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_PARALLEL_ACTIONS_H_
#define SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_PARALLEL_ACTIONS_H_

#include <action_graph/action.h>
#include <future>
#include <memory>
#include <vector>

namespace action_graph {
class ParallelActions final : public Action {
public:
  template <class... Actions>
  ParallelActions(std::string name, std::unique_ptr<Actions>... actions)
      : Action(std::move(name)) {
    (sequence_.emplace_back(std::move(actions)), ...);
  }

  void Execute() override {
    std::vector<std::future<void>> futures;

    for (auto &action : sequence_) {
      auto future =
          std::async(std::launch::async, [&action]() { action->Execute(); });
      futures.push_back(std::move(future));
    }

    for (auto &future : futures) {
      future.get(); // Wait for all actions to complete
    }
  }

private:
  std::vector<std::unique_ptr<Action>> sequence_;
};
} // namespace action_graph

#endif
