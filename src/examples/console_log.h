// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#ifndef ACTION_GRAPH_EXAMPLES_CONSOLE_LOG_H
#define ACTION_GRAPH_EXAMPLES_CONSOLE_LOG_H

#include <action_graph/log.h>

#include <mutex>
#include <ostream>
#include <string>

namespace action_graph_examples {

class ConsoleLog final : public action_graph::Log {
public:
  explicit ConsoleLog(std::ostream &output);

  void LogMessage(const std::string &message) override;
  void LogError(const std::string &message) override;

private:
  void WriteLine(const std::string &line);

  std::ostream &output_;
  std::mutex mutex_{};
};

} // namespace action_graph_examples

#endif // ACTION_GRAPH_EXAMPLES_CONSOLE_LOG_H
