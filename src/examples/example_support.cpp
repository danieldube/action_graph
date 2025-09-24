// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#include "example_support.h"

#include <action_graph/builder/builder.h>
#include <action_graph/builder/parse_duration.h>
#include <action_graph/single_action.h>

#include <chrono>
#include <exception>
#include <iomanip>
#include <sstream>
#include <thread>
#include <utility>
#include <vector>

namespace {

using ::action_graph::builder::ConfigurationNode;

bool TryGetScalar(const ConfigurationNode &node, const std::string &key,
                  std::string &value) {
  if (!node.HasKey(key)) {
    return false;
  }
  value = node.Get(key).AsString();
  return true;
}

std::string DescribeActionSummary(const ConfigurationNode &action_node) {
  std::string action_name;
  const bool has_name = TryGetScalar(action_node, "name", action_name);
  if (has_name) {
    action_name = "'" + action_name + "'";
  } else {
    action_name = "an unnamed action";
  }

  std::string action_type;
  if (!TryGetScalar(action_node, "type", action_type)) {
    return action_name;
  }

  std::ostringstream summary;
  summary << action_name << " [" << action_type << "]";

  if (action_type == "log_message") {
    std::string message;
    if (TryGetScalar(action_node, "message", message)) {
      summary << ": \"" << message << "\"";
    }
  } else if (action_type == "wait") {
    std::string duration;
    if (TryGetScalar(action_node, "duration", duration)) {
      summary << ": waits " << duration;
    }
  }

  return summary.str();
}

void DescribeActionTree(const ConfigurationNode &action_node,
                        examples::ExampleContext &context, int indent) {
  std::ostringstream line;
  line << std::string(static_cast<std::size_t>(indent), ' ') << "- "
       << DescribeActionSummary(action_node);
  context.Log(line.str());

  std::string action_type;
  if (!TryGetScalar(action_node, "type", action_type)) {
    return;
  }

  if (action_type == "sequential_actions" ||
      action_type == "parallel_actions") {
    if (!action_node.HasKey("actions")) {
      return;
    }

    const auto &actions = action_node.Get("actions");
    for (std::size_t index = 0; index < actions.Size(); ++index) {
      const auto &entry = actions.Get(index);
      if (!entry.HasKey("action")) {
        continue;
      }
      DescribeActionTree(entry.Get("action"), context, indent + 4);
    }
  }
}

struct TriggerOverview {
  std::string name;
  examples::SteadyClock::duration period{};
  bool has_period = false;
  std::string period_text;
  std::string action_summary;
  std::vector<std::string> decorators;
};

TriggerOverview BuildTriggerOverview(const ConfigurationNode &trigger_entry,
                                     examples::ExampleContext &context) {
  const auto &trigger_node = trigger_entry.Get("trigger");
  TriggerOverview overview;

  if (!TryGetScalar(trigger_node, "name", overview.name)) {
    overview.name = "(unnamed trigger)";
  }

  if (TryGetScalar(trigger_node, "period", overview.period_text)) {
    try {
      const auto parsed_period =
          action_graph::builder::ParseDuration(overview.period_text);
      overview.period =
          std::chrono::duration_cast<examples::SteadyClock::duration>(
              parsed_period);
      overview.has_period = true;
    } catch (const std::exception &error) {
      std::ostringstream warning;
      warning << "Unable to parse period for trigger '" << overview.name
              << "': " << error.what();
      context.LogError(warning.str());
    }
  }

  if (trigger_node.HasKey("action")) {
    overview.action_summary = DescribeActionSummary(trigger_node.Get("action"));
  } else {
    overview.action_summary = "(no action configured)";
  }

  if (trigger_node.HasKey("decorate")) {
    const auto &decorate_sequence = trigger_node.Get("decorate");
    for (std::size_t index = 0; index < decorate_sequence.Size(); ++index) {
      const auto &decorator = decorate_sequence.Get(index);
      std::string decorator_type;
      if (TryGetScalar(decorator, "type", decorator_type)) {
        overview.decorators.push_back(decorator_type);
      }
    }
  }

  return overview;
}

void LogTriggerOverview(const std::vector<TriggerOverview> &triggers,
                        examples::ExampleContext &context) {
  if (triggers.empty()) {
    context.Log("No triggers were scheduled by this configuration.");
    return;
  }

  context.Log("Configured triggers:");
  for (const auto &trigger : triggers) {
    std::ostringstream line;
    line << "  - '" << trigger.name << "' fires every ";
    if (trigger.has_period) {
      line << context.DescribeDuration(trigger.period);
    } else if (!trigger.period_text.empty()) {
      line << trigger.period_text;
    } else {
      line << "(unspecified period)";
    }
    line << " and executes " << trigger.action_summary;
    if (!trigger.decorators.empty()) {
      line << " (decorated by ";
      for (std::size_t index = 0; index < trigger.decorators.size(); ++index) {
        if (index > 0) {
          line << ", ";
        }
        line << trigger.decorators[index];
      }
      line << ')';
    }
    context.Log(line.str());
  }
}

void LogConfigurationOverview(
    const action_graph::yaml_cpp_configuration::Node &configuration,
    examples::ExampleContext &context) {
  if (configuration.IsSequence()) {
    std::vector<TriggerOverview> trigger_overviews;
    for (std::size_t index = 0; index < configuration.Size(); ++index) {
      const auto &entry = configuration.Get(index);
      if (!entry.HasKey("trigger")) {
        continue;
      }
      trigger_overviews.push_back(BuildTriggerOverview(entry, context));
    }
    LogTriggerOverview(trigger_overviews, context);
    return;
  }

  if (configuration.IsMap() && configuration.HasKey("action")) {
    context.Log("Configured action graph:");
    DescribeActionTree(configuration.Get("action"), context, 2);
    return;
  }

  context.Log("Configuration did not contain triggers or actions that could be "
              "summarised.");
}

} // namespace

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

  return builder;
}

action_graph::builder::GenericActionDecorator
CreateExampleActionDecorator(ExampleContext &context) {
  using action_graph::builder::ConfigurationNode;
  using action_graph::builder::DecorateWithTimingMonitor;

  action_graph::builder::GenericActionDecorator decorator;

  decorator.AddDecoratorFunction(
      "timing_monitor", [&context](const ConfigurationNode &node,
                                   action_graph::builder::ActionObject action) {
        std::ostringstream details;
        details << "Attaching timing monitor to '" << action->name
                << "' with a duration limit of "
                << node.Get("duration_limit").AsString()
                << " and an expected period of "
                << node.Get("expected_period").AsString();
        context.Log(details.str());
        return DecorateWithTimingMonitor<SteadyClock>(node, std::move(action),
                                                      context);
      });

  return decorator;
}

action_graph::builder::ActionObject BuildExampleAction(
    const action_graph::builder::ConfigurationNode &node,
    const action_graph::builder::GenericActionBuilder &builder,
    const action_graph::builder::GenericActionDecorator &decorator) {
  auto action = builder(node);
  action = decorator(node, std::move(action));
  if (node.HasKey("action")) {
    const auto &inner_action = node.Get("action");
    action = decorator(inner_action, std::move(action));
  }
  return action;
}

std::vector<action_graph::builder::ActionObject> BuildScheduledActions(
    const action_graph::yaml_cpp_configuration::Node &configuration,
    const action_graph::builder::GenericActionBuilder &builder,
    const action_graph::builder::GenericActionDecorator &decorator,
    ExampleContext &context, Timer &timer) {
  using action_graph::builder::ConfigurationError;

  std::vector<action_graph::builder::ActionObject> created_actions;
  for (std::size_t entry_index = 0; entry_index < configuration.Size();
       ++entry_index) {
    const auto &entry = configuration.Get(entry_index);
    if (!entry.HasKey("trigger")) {
      throw ConfigurationError("Only trigger nodes are allowed on top level.",
                               entry);
    }

    const auto &trigger = entry.Get("trigger");
    const auto trigger_name = trigger.Get("name").AsString();
    const auto trigger_period_string = trigger.Get("period").AsString();
    const auto trigger_period =
        action_graph::builder::ParseDuration(trigger_period_string);
    const auto casted_trigger_period =
        std::chrono::duration_cast<SteadyClock::duration>(trigger_period);

    auto action = BuildExampleAction(trigger, builder, decorator);
    auto *action_ptr = action.get();

    timer.SetTriggerTime(casted_trigger_period,
                         [action_ptr]() { action_ptr->Execute(); });

    std::ostringstream overview;
    overview << "Scheduled trigger '" << trigger_name << "' every "
             << trigger_period_string << ".";
    context.Log(overview.str());

    created_actions.push_back(std::move(action));
  }
  return created_actions;
}

ExampleSession::ExampleSession(std::ostream &out, std::string title,
                               std::string configuration_yaml)
    : title_(std::move(title)),
      configuration_yaml_(std::move(configuration_yaml)), context_(out),
      configuration_(
          action_graph::yaml_cpp_configuration::Node::CreateFromString(
              configuration_yaml_)),
      action_builder_(CreateExampleActionBuilder(context_)),
      decorator_builder_(CreateExampleActionDecorator(context_)) {
  context_.Log(std::string{"\n=== "} + title_ + " ===");
  context_.Log("Configuration YAML:");

  std::istringstream configuration_stream(configuration_yaml_);
  std::string line;
  while (std::getline(configuration_stream, line)) {
    context_.Log("  " + line);
  }
  context_.Log("");
  LogConfigurationOverview(configuration_, context_);
}

ExampleSession::~ExampleSession() { context_.PrintSummary(title_); }

ExampleContext &ExampleSession::Context() { return context_; }

const ExampleContext &ExampleSession::Context() const { return context_; }

const std::string &ExampleSession::Title() const { return title_; }

const action_graph::yaml_cpp_configuration::Node &
ExampleSession::Configuration() const {
  return configuration_;
}

action_graph::builder::GenericActionBuilder &ExampleSession::ActionBuilder() {
  return action_builder_;
}

const action_graph::builder::GenericActionBuilder &
ExampleSession::ActionBuilder() const {
  return action_builder_;
}

action_graph::builder::GenericActionDecorator &ExampleSession::Decorator() {
  return decorator_builder_;
}

const action_graph::builder::GenericActionDecorator &
ExampleSession::Decorator() const {
  return decorator_builder_;
}

action_graph::builder::ActionObject ExampleSession::BuildAction(
    const action_graph::builder::ConfigurationNode &node) const {
  return BuildExampleAction(node, ActionBuilder(), Decorator());
}

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
