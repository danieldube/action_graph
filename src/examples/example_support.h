// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#pragma once

#include <action_graph/builder/generic_action_builder.h>
#include <action_graph/global_timer/global_timer.h>

#include <chrono>
#include <iosfwd>
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace examples {

using SteadyClock = std::chrono::steady_clock;
using Timer = action_graph::GlobalTimer<SteadyClock>;

class ExampleContext {
public:
  explicit ExampleContext(std::ostream &out);

  void Log(const std::string &message);
  SteadyClock::duration RecordExecution(const std::string &action_name);
  void PrintSummary(const std::string &example_title);
  std::string DescribeOffset(SteadyClock::duration offset) const;
  template <typename Rep, typename Period>
  std::string
  DescribeDuration(std::chrono::duration<Rep, Period> duration) const {
    return DescribeDurationImpl(
        std::chrono::duration_cast<SteadyClock::duration>(duration));
  }
  std::string DescribeDuration(SteadyClock::duration duration) const;

private:
  struct ActionStats {
    std::size_t count = 0;
    std::vector<SteadyClock::duration> offsets;
  };

  std::string FormatDuration(SteadyClock::duration duration) const;
  std::string DescribeDurationImpl(SteadyClock::duration duration) const;

  std::ostream &out_;
  SteadyClock::time_point start_time_;
  std::mutex mutex_;
  std::map<std::string, ActionStats> action_stats_;
  std::vector<std::string> action_order_;
};

action_graph::builder::GenericActionBuilder
CreateExampleActionBuilder(ExampleContext &context);

} // namespace examples
