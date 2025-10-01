// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.
#ifndef SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_DECORATORS_DECORATED_ACTION_H_
#define SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_DECORATORS_DECORATED_ACTION_H_

#include <action_graph/action.h>
#include <memory>
#include <stdexcept>

namespace action_graph {
namespace decorators {
class DecoratedAction : public action_graph::Action {
public:
  using Action = action_graph::Action;
  explicit DecoratedAction(std::unique_ptr<Action> action)
      : Action(action->name), action_(std::move(action)) {
    if (!action_) {
      throw std::invalid_argument("The Action is nullptr.");
    }
  }

protected:
  Action &GetAction() { return *action_; }
  const Action &GetAction() const { return *action_; }

private:
  std::unique_ptr<Action> action_;
};
} // namespace decorators
} // namespace action_graph
#endif // SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_DECORATORS_DECORATED_ACTION_H_
