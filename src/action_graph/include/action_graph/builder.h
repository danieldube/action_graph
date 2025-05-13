#ifndef ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_BUILDER_H_
#define ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_BUILDER_H_

#include <action_graph/action.h>
#include <functional>
#include <map>
#include <memory>
#include <yaml-cpp/yaml.h>

namespace action_graph {
namespace builder {
using ::action_graph::Action;
using ActionObject = std::unique_ptr<Action>;
using CreatorFunction = std::function<ActionObject(const YAML::Node &)>;
using ActionCreators = std::map<std::string, CreatorFunction>;

std::vector<ActionObject>
BuildActionGraph(const std::string &yaml_string,
                 const ActionCreators &action_creators);
} // namespace builder
} // namespace action_graph

#endif // ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_BUILDER_H_
