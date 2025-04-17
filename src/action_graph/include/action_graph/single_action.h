#ifndef SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_SINGLE_ACTION_H_
#define SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_SINGLE_ACTION_H_

#include <action_graph/action.h>

namespace action_graph {
class SingleAction final : public Action {
private:
  void Execute() override {}
};
} // namespace action_graph

#endif // SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_SINGLE_ACTION_H_
