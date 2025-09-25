// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#ifndef ACTION_GRAPH_SRC_EXAMPLES_EXAMPLE_CONTEXT_H_
#define ACTION_GRAPH_SRC_EXAMPLES_EXAMPLE_CONTEXT_H_

#include <action_graph/log.h>

#include <cstddef>
#include <iosfwd>
#include <map>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

class ExampleContext : public action_graph::Log {
public:
  explicit ExampleContext(std::ostream &output) noexcept;

  void LogMessage(const std::string &message) override;
  void LogError(const std::string &message) override;
  std::size_t RecordExecution(const std::string &action_name);
  void PrintSummary(std::string_view title) const;

private:
  void PrintExecutionCounts() const;
  void PrintErrors() const;

  std::ostream &output_;
  mutable std::mutex mutex_;
  std::vector<std::string> messages_;
  std::map<std::string, std::size_t> error_counts_;
  std::map<std::string, std::size_t> execution_counts_;
};

#endif // ACTION_GRAPH_SRC_EXAMPLES_EXAMPLE_CONTEXT_H_
