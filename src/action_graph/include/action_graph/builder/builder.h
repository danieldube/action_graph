#ifndef ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_BUILDER_BUILDER_H_
#define ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_BUILDER_BUILDER_H_

#include "action_graph/action.h"
#include "yaml-cpp/yaml.h"
#include <chrono>
#include <functional>
#include <map>
#include <memory>

namespace action_graph {
namespace builder {
using ::action_graph::Action;
using ActionObject = std::unique_ptr<Action>;
class ActionBuilder;
using BuilderFunction =
    std::function<ActionObject(const YAML::Node &, const ActionBuilder &)>;
using BuilderFunctions = std::map<std::string, BuilderFunction>;

class ActionBuilder {
public:
  explicit ActionBuilder(BuilderFunctions builder_functions);
  ActionObject operator()(const YAML::Node &node) const;

private:
  BuilderFunctions builder_functions_;
};

class YamlParsingError : public std::runtime_error {
public:
  explicit YamlParsingError(const std::string &message, const YAML::Node &node)
      : std::runtime_error("Error parsing yaml node: " + message + " :\n" +
                           YAML::Dump(node)) {}
};

class BuildError : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

std::vector<ActionObject>
BuildActionGraph(const std::string &yaml_string,
                 const BuilderFunctions &action_builders);

ActionObject BuildTrigger(const YAML::Node &yaml_node,
                          const BuilderFunctions &action_builders);
} // namespace builder
} // namespace action_graph

#endif // ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_BUILDER_BUILDER_H_
