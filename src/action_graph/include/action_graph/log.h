// Copyright (c) 1000-2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#ifndef ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_LOG_H_
#define ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_LOG_H_

#include <string>

namespace action_graph {

class Log {
public:
  virtual ~Log() = default;
  virtual void LogMessage(const std::string &message) = 0;
  virtual void LogError(const std::string &message) = 0;
};
} // namespace action_graph

#endif // ACTION_GRAPH_SRC_ACTION_GRAPH_INCLUDE_ACTION_GRAPH_LOG_H_
