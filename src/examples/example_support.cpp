// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#include "example_support.h"

#include <action_graph/builder/builder.h>
#include <action_graph/builder/generic_action_decorator.h>
#include <action_graph/builder/parse_duration.h>
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

void ExampleContext::LogMessage(const std::string &message) { Log(message); }

void ExampleContext::LogError(const std::string &message) {
  Log("Error: " + message);
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

ExampleActionBuilder::ExampleActionBuilder(ExampleContext &context)
    : action_builder_(action_graph::builder::
                          CreateGenericActionBuilderWithDefaultActions()),
      context_(&context) {
  using action_graph::builder::ConfigurationNode;
  using action_graph::builder::DecorateWithTimingMonitor;

  decorator_builder_.AddDecoratorFunction(
      "timing_monitor", [this](const ConfigurationNode &node,
                               action_graph::builder::ActionObject action) {
        if (context_ != nullptr) {
          std::ostringstream details;
          details << "Attaching timing monitor to '" << action->name
                  << "' with a duration limit of "
                  << node.Get("duration_limit").AsString()
                  << " and an expected period of "
                  << node.Get("expected_period").AsString();
          context_->Log(details.str());
        }
        return DecorateWithTimingMonitor<SteadyClock>(node, std::move(action),
                                                      *context_);
      });
}

action_graph::builder::ActionObject ExampleActionBuilder::operator()(
    const action_graph::builder::ConfigurationNode &node) const {
  auto action = action_builder_(node);
  return decorator_builder_(node, std::move(action));
}

action_graph::builder::GenericActionBuilder &ExampleActionBuilder::Actions() {
  return action_builder_;
}

const action_graph::builder::GenericActionBuilder &
ExampleActionBuilder::Actions() const {
  return action_builder_;
}

action_graph::builder::GenericActionDecorator &
ExampleActionBuilder::Decorators() {
  return decorator_builder_;
}

const action_graph::builder::GenericActionDecorator &
ExampleActionBuilder::Decorators() const {
  return decorator_builder_;
}

ExampleActionBuilder CreateExampleActionBuilder(ExampleContext &context) {
  using action_graph::builder::ActionBuilder;
  using action_graph::builder::ConfigurationNode;

  ExampleActionBuilder builder(context);

  builder.Actions().AddBuilderFunction(
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

  builder.Actions().AddBuilderFunction(
      "wait", [&context](const ConfigurationNode &node, const ActionBuilder &) {
        const auto name = node.Get("name").AsString();
        const auto duration_string = node.Get("duration").AsString();
        const auto parsed_duration =
            action_graph::builder::ParseDuration(duration_string);
        const auto wait_duration =
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                parsed_duration);
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

  return builder;
}

ExampleSession::ExampleSession(std::ostream &out, std::string title,
                               std::string configuration_yaml)
    : title_(std::move(title)),
      configuration_yaml_(std::move(configuration_yaml)), context_(out),
      configuration_(
          action_graph::yaml_cpp_configuration::Node::CreateFromString(
              configuration_yaml_)),
      builder_(CreateExampleActionBuilder(context_)) {
  context_.Log(std::string{"\n=== "} + title_ + " ===");
  context_.Log("Configuration YAML:");

  std::istringstream configuration_stream(configuration_yaml_);
  std::string line;
  while (std::getline(configuration_stream, line)) {
    context_.Log("  " + line);
  }
  context_.Log("");
}

ExampleSession::~ExampleSession() { context_.PrintSummary(title_); }

ExampleContext &ExampleSession::Context() { return context_; }

const ExampleContext &ExampleSession::Context() const { return context_; }

const std::string &ExampleSession::Title() const { return title_; }

const action_graph::yaml_cpp_configuration::Node &
ExampleSession::Configuration() const {
  return configuration_;
}

ExampleActionBuilder &ExampleSession::Builder() { return builder_; }

const ExampleActionBuilder &ExampleSession::Builder() const { return builder_; }

std::string DescribeCount(std::size_t count, const std::string &singular,
                          const std::string &plural) {
  std::ostringstream stream;
  stream << count << ' ' << (count == 1 ? singular : plural);
  return stream.str();
}

void ObserveForDuration(ExampleContext &context, Timer &timer,
                        SteadyClock::duration duration,
                        const std::string &reason) {
  std::ostringstream message;
  message << reason << " for " << context.DescribeDuration(duration) << "...";
  context.Log(message.str());
  std::this_thread::sleep_for(duration);
  timer.WaitOneCycle();
}

} // namespace examples
