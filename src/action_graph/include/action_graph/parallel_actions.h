// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#ifndef SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_PARALLEL_ACTIONS_H_
#define SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_PARALLEL_ACTIONS_H_

#include <action_graph/action.h>
#include <future>
#include <memory>
#include <utility>
#include <vector>

namespace action_graph {
class ParallelActions final : public Action {
public:
  template <class... Actions>
  ParallelActions(std::string name, std::unique_ptr<Actions>... actions)
      : Action(std::move(name)) {
    AppendActions(std::move(actions)...);
  }

  ParallelActions(std::string name,
                  std::vector<std::unique_ptr<Action>> actions)
      : Action(std::move(name)), sequence_(std::move(actions)) {}

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
  static void AppendActions() {}

  template <typename ActionPtr, typename... Remaining>
  void AppendActions(ActionPtr action, Remaining... remaining) {
    sequence_.emplace_back(std::move(action));
    AppendActions(std::move(remaining)...);
  }

  std::vector<std::unique_ptr<Action>> sequence_;
};
} // namespace action_graph

#endif
