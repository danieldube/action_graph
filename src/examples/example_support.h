// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#pragma once

#include <action_graph/builder/generic_action_builder.h>
#include <action_graph/builder/generic_action_decorator.h>
#include <action_graph/global_timer/global_timer.h>
#include <action_graph/log.h>
#include <yaml_cpp_configuration/yaml_node.h>

#include <chrono>
#include <cstddef>
#include <functional>
#include <iosfwd>
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace examples {

using SteadyClock = std::chrono::steady_clock;
using Timer = action_graph::GlobalTimer<SteadyClock>;

class ExampleContext : public action_graph::Log {
public:
  explicit ExampleContext(std::ostream &out);

  void Log(const std::string &message);
  SteadyClock::duration RecordExecution(const std::string &action_name);
  void PrintSummary(const std::string &example_title);
  std::string DescribeOffset(SteadyClock::duration offset) const;
  template <typename Rep, typename Period>
  std::string
  DescribeDuration(std::chrono::duration<Rep, Period> duration) const {
    return FormatDuration(
        std::chrono::duration_cast<SteadyClock::duration>(duration));
  }
  std::string DescribeDuration(SteadyClock::duration duration) const;

  void LogMessage(const std::string &message) override;
  void LogError(const std::string &message) override;

private:
  struct ActionStats {
    std::size_t count = 0;
    std::vector<SteadyClock::duration> offsets;
  };

  std::string FormatDuration(SteadyClock::duration duration) const;
  std::ostream &out_;
  SteadyClock::time_point start_time_;
  std::mutex mutex_;
  std::map<std::string, ActionStats> action_stats_;
  std::vector<std::string> action_order_;
};

class ExampleActionBuilder final : public action_graph::builder::ActionBuilder {
public:
  explicit ExampleActionBuilder(ExampleContext &context);
  ExampleActionBuilder(const ExampleActionBuilder &) = delete;
  ExampleActionBuilder &operator=(const ExampleActionBuilder &) = delete;
  ExampleActionBuilder(ExampleActionBuilder &&) noexcept = default;
  ExampleActionBuilder &operator=(ExampleActionBuilder &&) noexcept = default;

  action_graph::builder::ActionObject operator()(
      const action_graph::builder::ConfigurationNode &node) const override;

  action_graph::builder::GenericActionBuilder &Actions();
  const action_graph::builder::GenericActionBuilder &Actions() const;

  action_graph::builder::GenericActionDecorator &Decorators();
  const action_graph::builder::GenericActionDecorator &Decorators() const;

private:
  action_graph::builder::GenericActionBuilder action_builder_;
  action_graph::builder::GenericActionDecorator decorator_builder_;
  std::reference_wrapper<ExampleContext> context_;
};

ExampleActionBuilder CreateExampleActionBuilder(ExampleContext &context);

class ExampleSession {
public:
  ExampleSession(std::ostream &out, std::string title,
                 std::string configuration_yaml);
  ~ExampleSession();

  ExampleSession(const ExampleSession &) = delete;
  ExampleSession &operator=(const ExampleSession &) = delete;
  ExampleSession(ExampleSession &&) = delete;
  ExampleSession &operator=(ExampleSession &&) = delete;

  ExampleContext &Context();
  const ExampleContext &Context() const;

  const std::string &Title() const;
  const action_graph::yaml_cpp_configuration::Node &Configuration() const;
  ExampleActionBuilder &Builder();
  const ExampleActionBuilder &Builder() const;

private:
  std::string title_;
  std::string configuration_yaml_;
  ExampleContext context_;
  action_graph::yaml_cpp_configuration::Node configuration_;
  ExampleActionBuilder builder_;
};

std::string DescribeCount(std::size_t count, const std::string &singular,
                          const std::string &plural);

void ObserveForDuration(ExampleContext &context, Timer &timer,
                        SteadyClock::duration duration,
                        const std::string &reason);

} // namespace examples
