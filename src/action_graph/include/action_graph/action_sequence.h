// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#ifndef SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_ACTION_SEQUENCE_H_
#define SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_ACTION_SEQUENCE_H_

#include <action_graph/action.h>
#include <atomic>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

namespace action_graph {

class ActionSequence final : public Action {
public:
  using Action::Action;

  template <class... Actions>
  explicit ActionSequence(std::string name, std::unique_ptr<Actions>... actions)
      : Action(std::move(name)) {
    AppendActions(std::move(actions)...);
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
  void AppendActions() {}

  template <typename ActionPtr, typename... Remaining>
  void AppendActions(ActionPtr action, Remaining... remaining) {
    sequence_.emplace_back(std::move(action));
    AppendActions(std::move(remaining)...);
  }

  std::vector<std::unique_ptr<Action>> sequence_;
};
} // namespace action_graph

#endif
