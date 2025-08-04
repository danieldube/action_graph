#ifndef ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_DECORATED_ACTION_H_
#define ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_DECORATED_ACTION_H_

#include <action_graph/action.h>
#include <memory>
#include <stdexcept>

namespace action_graph {
class DecoratedAction : public Action {
public:
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
} // namespace action_graph
#endif // ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_DECORATED_ACTION_H_
