// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#include "example_support.h"

#include <action_graph/builder/builder.h>
#include <action_graph/builder/parse_duration.h>
#include <action_graph/decorators/timing_monitor.h>
#include <action_graph/single_action.h>

#include <chrono>
#include <iomanip>
#include <sstream>
#include <thread>
#include <utility>

namespace examples {

ExampleContext::ExampleContext(std::ostream &out)
    : out_(out), start_time_(SteadyClock::now()) {}

void ExampleContext::Log(const std::string &message) {
  std::lock_guard<std::mutex> lock(mutex_);
  out_ << message << std::endl;
}

SteadyClock::duration
ExampleContext::RecordExecution(const std::string &action_name) {
  const auto now = SteadyClock::now();
  const auto offset = now - start_time_;

  std::lock_guard<std::mutex> lock(mutex_);
  auto &stats = action_stats_[action_name];
  ++stats.count;
  stats.offsets.push_back(offset);
  if (stats.count == 1) {
    action_order_.push_back(action_name);
  }
  return offset;
}

void ExampleContext::PrintSummary(const std::string &example_title) {
  std::vector<std::string> lines;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    lines.emplace_back("\n--- Summary for " + example_title + " ---");
    if (action_order_.empty()) {
      lines.emplace_back("  (No actions were executed.)");
    } else {
      for (const auto &name : action_order_) {
        const auto stats_iterator = action_stats_.find(name);
        if (stats_iterator == action_stats_.end()) {
          continue;
        }
        const auto &stats = stats_iterator->second;

        std::ostringstream line;
        line << "  - " << name << ": executed " << stats.count << " time"
             << (stats.count == 1 ? "" : "s");

        if (!stats.offsets.empty()) {
          line << ", first at " << DescribeOffset(stats.offsets.front());
          line << ", last at " << DescribeOffset(stats.offsets.back());
        }

        if (stats.offsets.size() > 1) {
          SteadyClock::duration total_interval{};
          for (std::size_t i = 1; i < stats.offsets.size(); ++i) {
            total_interval += stats.offsets[i] - stats.offsets[i - 1];
          }
          const auto average_interval =
              total_interval /
              static_cast<SteadyClock::duration::rep>(stats.offsets.size() - 1);
          line << ", avg interval " << DescribeDuration(average_interval);
        }

        lines.push_back(line.str());
      }
    }
  }

  for (const auto &line : lines) {
    out_ << line << std::endl;
  }
}

std::string
ExampleContext::FormatDuration(SteadyClock::duration duration) const {
  const auto as_milliseconds =
      std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(
          duration)
          .count();

  std::ostringstream stream;
  stream << std::fixed << std::setprecision(2);

  if (as_milliseconds >= 1000.0) {
    stream << as_milliseconds / 1000.0 << " s";
    return stream.str();
  }

  if (as_milliseconds < 1.0) {
    const auto as_microseconds =
        std::chrono::duration_cast<std::chrono::duration<double, std::micro>>(
            duration)
            .count();
    stream << as_microseconds << " µs";
    return stream.str();
  }

  stream << as_milliseconds << " ms";
  return stream.str();
}

std::string ExampleContext::DescribeOffset(SteadyClock::duration offset) const {
  return std::string{"+"} + FormatDuration(offset);
}

std::string
ExampleContext::DescribeDuration(SteadyClock::duration duration) const {
  return DescribeDurationImpl(duration);
}

std::string
ExampleContext::DescribeDurationImpl(SteadyClock::duration duration) const {
  return FormatDuration(duration);
}

action_graph::builder::GenericActionBuilder
CreateExampleActionBuilder(ExampleContext &context) {
  using action_graph::builder::ActionBuilder;
  using action_graph::builder::ConfigurationNode;

  auto builder =
      action_graph::builder::CreateGenericActionBuilderWithDefaultActions();

  builder.AddBuilderFunction(
      "log_message",
      [&context](const ConfigurationNode &node, const ActionBuilder &) {
        const auto name = node.Get("name").AsString();
        if (!node.HasKey("message")) {
          std::ostringstream error;
          error << "Configuration for log_message '" << name
                << "' is missing a message field:\n"
                << node.AsString();
          context.Log(error.str());
          throw action_graph::builder::ConfigurationError(
              "log_message action requires a message.", node);
        }
        const auto message = node.Get("message").AsString();
        return std::make_unique<action_graph::SingleAction>(
            name, [&context, name, message]() {
              const auto offset = context.RecordExecution(name);
              std::ostringstream output;
              output << name << " (" << context.DescribeOffset(offset)
                     << "): " << message;
              context.Log(output.str());
            });
      });

  builder.AddBuilderFunction("wait", [&context](const ConfigurationNode &node,
                                                const ActionBuilder &) {
    const auto name = node.Get("name").AsString();
    const auto duration_string = node.Get("duration").AsString();
    const auto parsed_duration =
        action_graph::builder::ParseDuration(duration_string);
    const auto wait_duration =
        std::chrono::duration_cast<std::chrono::nanoseconds>(parsed_duration);
    return std::make_unique<action_graph::SingleAction>(
        name, [name, duration_string, wait_duration, &context]() {
          const auto offset = context.RecordExecution(name);
          std::ostringstream start_message;
          start_message << name << " (" << context.DescribeOffset(offset)
                        << "): simulating work for " << duration_string;
          context.Log(start_message.str());

          const auto start_time = SteadyClock::now();
          std::this_thread::sleep_for(wait_duration);
          const auto elapsed = SteadyClock::now() - start_time;

          std::ostringstream finish_message;
          finish_message << name << ": finished simulated work in "
                         << context.DescribeDuration(elapsed);
          context.Log(finish_message.str());
        });
  });

  builder.AddBuilderFunction(
      "monitored_action", [&context](const ConfigurationNode &node,
                                     const ActionBuilder &action_builder) {
        using Monitor = action_graph::decorators::TimingMonitor<SteadyClock>;

        const auto name = node.Get("name").AsString();
        const auto &monitor_configuration = node.Get("monitor");
        const auto duration_limit = action_graph::builder::ParseDuration(
            monitor_configuration.Get("duration_limit").AsString());
        const auto trigger_period = action_graph::builder::ParseDuration(
            monitor_configuration.Get("period").AsString());
        const auto on_duration_exceeded_message =
            monitor_configuration.Get("on_duration_exceeded").AsString();
        const auto on_trigger_miss_message =
            monitor_configuration.Get("on_trigger_miss").AsString();

        auto monitored_action = action_builder(node.Get("action"));

        const auto converted_duration_limit =
            std::chrono::duration_cast<Monitor::Duration>(duration_limit);
        const auto converted_period =
            std::chrono::duration_cast<Monitor::Duration>(trigger_period);

        return std::make_unique<Monitor>(
            std::move(monitored_action), converted_duration_limit,
            [name, on_duration_exceeded_message, &context]() {
              std::ostringstream message;
              message << name << ": " << on_duration_exceeded_message;
              context.Log(message.str());
            },
            converted_period,
            [name, on_trigger_miss_message, &context]() {
              std::ostringstream message;
              message << name << ": " << on_trigger_miss_message;
              context.Log(message.str());
            });
      });

  return builder;
}

} // namespace examples
