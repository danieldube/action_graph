#ifndef ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_ACTION_H_
#define ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_ACTION_H_

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

#endif // ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_ACTION_H_
