// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.
#ifndef SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_SINGLE_ACTION_H_
#define SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_SINGLE_ACTION_H_

#include <action_graph/action.h>
#include <functional>

namespace action_graph {
class SingleAction final : public Action {
public:
  SingleAction(std::string name, std::function<void()> function)
      : Action(std::move(name)), function_{std::move(function)} {}

  void Execute() override { function_(); }

private:
  std::function<void()> function_;
};
} // namespace action_graph

#endif // SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_SINGLE_ACTION_H_
