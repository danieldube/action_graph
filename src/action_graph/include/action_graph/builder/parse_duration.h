#ifndef ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_BILDER_DURATION_PARSER_H_
#define ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_BILDER_DURATION_PARSER_H_

#include "action_graph/action.h"
#include "yaml-cpp/yaml.h"
#include <chrono>
#include <functional>
#include <map>
#include <memory>

namespace action_graph {
namespace builder {
std::chrono::duration<double> ParseDuration(const std::string &duration_str);
} // namespace builder
} // namespace action_graph
#endif // ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_BILDER_DURATION_PARSER_H_
