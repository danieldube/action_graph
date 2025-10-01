// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.
#ifndef SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_ACTION_H_
#define SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_ACTION_H_

#include <string>

namespace action_graph {

class Action {
public:
  explicit Action(std::string name) : name(std::move(name)) {}

  virtual ~Action() = default;

  virtual void Execute() = 0;

  const std::string name;
};
} // namespace action_graph

#endif // SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_ACTION_H_
