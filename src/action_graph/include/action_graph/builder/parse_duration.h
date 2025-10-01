// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.
#ifndef SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_BUILDER_PARSE_DURATION_H_
#define SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_BUILDER_PARSE_DURATION_H_

#include "action_graph/action.h"
#include <chrono>
#include <functional>
#include <map>
#include <memory>

namespace action_graph {
namespace builder {
std::chrono::duration<double> ParseDuration(const std::string &duration_str);
} // namespace builder
} // namespace action_graph
#endif // SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_BUILDER_PARSE_DURATION_H_
