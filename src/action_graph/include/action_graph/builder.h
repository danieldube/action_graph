#ifndef ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_BUILDER_H_
#define ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_BUILDER_H_

#include <action_graph/action.h>
#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <yaml-cpp/yaml.h>

namespace action_graph {
namespace builder {
using ::action_graph::Action;
using ActionObject = std::unique_ptr<Action>;
using BuilderFunction = std::function<ActionObject(const YAML::Node &)>;
using ActionBuilders = std::map<std::string, BuilderFunction>;

std::vector<ActionObject>
BuildActionGraph(const std::string &yaml_string,
                 const ActionBuilders &action_builders);

ActionObject BuildTrigger(const YAML::Node &yaml_node,
                          const ActionBuilders &action_builders);

std::chrono::duration<double> ParseDuration(const std::string &duration_str);
} // namespace builder
} // namespace action_graph

#endif // ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_BUILDER_H_
