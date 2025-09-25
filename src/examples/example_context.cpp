// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#include "example_context.h"

#include <ostream>

ExampleContext::ExampleContext(std::ostream &output) noexcept
    : output_(output) {}

void ExampleContext::LogMessage(const std::string &message) {
  std::lock_guard lock(mutex_);
  messages_.push_back(message);
  output_ << "  " << message << '\n';
}

void ExampleContext::LogError(const std::string &message) {
  std::lock_guard lock(mutex_);
  auto &count = error_counts_[message];
  ++count;
  output_ << "  Error: " << message;
  if (count > 1) {
    output_ << " (x" << count << ')';
  }
  output_ << '\n';
}

std::size_t ExampleContext::RecordExecution(const std::string &action_name) {
  std::lock_guard lock(mutex_);
  auto &count = execution_counts_[action_name];
  ++count;
  return count;
}

void ExampleContext::PrintSummary(std::string_view title) const {
  std::lock_guard lock(mutex_);
  output_ << "\nSummary: " << title << '\n';
  PrintExecutionCounts();
  PrintErrors();
}

void ExampleContext::PrintExecutionCounts() const {
  if (execution_counts_.empty()) {
    output_ << "  No actions executed." << '\n';
    return;
  }
  for (const auto &[name, count] : execution_counts_) {
    output_ << "  " << name << " executed " << count << " times" << '\n';
  }
}

void ExampleContext::PrintErrors() const {
  if (error_counts_.empty()) {
    return;
  }
  output_ << "  Reported issues:" << '\n';
  for (const auto &[error, count] : error_counts_) {
    output_ << "    - " << error;
    if (count > 1) {
      output_ << " (x" << count << ')';
    }
    output_ << '\n';
  }
}
