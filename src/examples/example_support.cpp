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

std::string GetScalarOrEmpty(const ConfigurationNode &node,
                             const std::string &key) {
  if (!node.HasKey(key)) {
    return {};
  }
  return node.Get(key).AsString();
}

std::string FormatActionName(const ConfigurationNode &action_node) {
  const auto action_name = GetScalarOrEmpty(action_node, "name");
  if (action_name.empty()) {
    return "an unnamed action";
  }
  return "'" + action_name + "'";
}

std::string BuildLogMessageDetail(const ConfigurationNode &action_node) {
  const auto message = GetScalarOrEmpty(action_node, "message");
  if (message.empty()) {
    return {};
  }
  return ": \"" + message + "\"";
}

std::string BuildWaitDetail(const ConfigurationNode &action_node) {
  const auto duration = GetScalarOrEmpty(action_node, "duration");
  if (duration.empty()) {
    return {};
  }
  return ": waits " + duration;
}

std::string BuildActionDetail(const std::string &action_type,
                              const ConfigurationNode &action_node) {
  if (action_type == "log_message") {
    return BuildLogMessageDetail(action_node);
  }
  if (action_type == "wait") {
    return BuildWaitDetail(action_node);
  }
  return {};
}

std::string ComposeSummary(const std::string &action_name,
                           const std::string &action_type,
                           const std::string &detail) {
  std::ostringstream summary;
  summary << action_name;
  if (action_type.empty()) {
    return summary.str();
  }
  summary << " [" << action_type << "]";
  summary << detail;
  return summary.str();
}

std::string DescribeActionSummary(const ConfigurationNode &action_node) {
  const auto action_name = FormatActionName(action_node);
  const auto action_type = GetScalarOrEmpty(action_node, "type");
  if (action_type.empty()) {
    return action_name;
  }
  const auto detail = BuildActionDetail(action_type, action_node);
  return ComposeSummary(action_name, action_type, detail);
}

void LogActionEntry(const ConfigurationNode &action_node,
                    examples::ExampleContext &context, int indent) {
  std::ostringstream line;
  line << std::string(static_cast<std::size_t>(indent), ' ') << "- "
       << DescribeActionSummary(action_node);
  context.Log(line.str());
}

bool IsCompositeType(const std::string &action_type) {
  return action_type == "sequential_actions" ||
         action_type == "parallel_actions";
}

void DescribeActionTree(const ConfigurationNode &action_node,
                        examples::ExampleContext &context, int indent);

void DescribeCompositeChildren(const ConfigurationNode &action_node,
                               examples::ExampleContext &context, int indent) {
  const auto &actions = action_node.Get("actions");
  for (std::size_t index = 0; index < actions.Size(); ++index) {
    const auto &entry = actions.Get(index);
    if (!entry.HasKey("action")) {
      continue;
    }
    DescribeActionTree(entry.Get("action"), context, indent + 4);
  }
}

void DescribeActionTree(const ConfigurationNode &action_node,
                        examples::ExampleContext &context, int indent) {
  LogActionEntry(action_node, context, indent);
  const auto action_type = GetScalarOrEmpty(action_node, "type");
  if (!IsCompositeType(action_type)) {
    return;
  }
  if (!action_node.HasKey("actions")) {
    return;
  }
  DescribeCompositeChildren(action_node, context, indent);
}

struct TriggerOverview {
  std::string name;
  examples::SteadyClock::duration period{};
  bool has_period = false;
  std::string period_text;
  std::string action_summary;
  std::vector<std::string> decorators;
};

std::string ExtractTriggerName(const ConfigurationNode &trigger_node) {
  const auto name = GetScalarOrEmpty(trigger_node, "name");
  if (!name.empty()) {
    return name;
  }
  return "(unnamed trigger)";
}

std::string ExtractPeriodText(const ConfigurationNode &trigger_node) {
  return GetScalarOrEmpty(trigger_node, "period");
}

void LogPeriodParseError(examples::ExampleContext &context,
                         const std::string &trigger_name,
                         const std::exception &error) {
  std::ostringstream warning;
  warning << "Unable to parse period for trigger '" << trigger_name
          << "': " << error.what();
  context.LogError(warning.str());
}

examples::SteadyClock::duration
TryParsePeriod(const std::string &period_text, bool &has_period,
               examples::ExampleContext &context,
               const std::string &trigger_name) {
  try {
    const auto parsed = action_graph::builder::ParseDuration(period_text);
    has_period = true;
    return std::chrono::duration_cast<examples::SteadyClock::duration>(parsed);
  } catch (const std::exception &error) {
    LogPeriodParseError(context, trigger_name, error);
    has_period = false;
    return {};
  }
}

examples::SteadyClock::duration
ParseTriggerPeriod(const std::string &period_text, bool &has_period,
                   examples::ExampleContext &context,
                   const std::string &trigger_name) {
  if (period_text.empty()) {
    has_period = false;
    return {};
  }
  return TryParsePeriod(period_text, has_period, context, trigger_name);
}

std::string BuildTriggerActionSummary(const ConfigurationNode &trigger_node) {
  if (!trigger_node.HasKey("action")) {
    return "(no action configured)";
  }
  return DescribeActionSummary(trigger_node.Get("action"));
}

std::vector<std::string>
CollectDecorators(const ConfigurationNode &trigger_node) {
  std::vector<std::string> decorators;
  if (!trigger_node.HasKey("decorate")) {
    return decorators;
  }
  const auto &decorate_sequence = trigger_node.Get("decorate");
  for (std::size_t index = 0; index < decorate_sequence.Size(); ++index) {
    const auto &decorator = decorate_sequence.Get(index);
    const auto decorator_type = GetScalarOrEmpty(decorator, "type");
    if (!decorator_type.empty()) {
      decorators.push_back(decorator_type);
    }
  }
  return decorators;
}

TriggerOverview BuildTriggerOverview(const ConfigurationNode &trigger_entry,
                                     examples::ExampleContext &context) {
  const auto &trigger_node = trigger_entry.Get("trigger");
  TriggerOverview overview;
  overview.name = ExtractTriggerName(trigger_node);
  overview.period_text = ExtractPeriodText(trigger_node);
  overview.period = ParseTriggerPeriod(
      overview.period_text, overview.has_period, context, overview.name);
  overview.action_summary = BuildTriggerActionSummary(trigger_node);
  overview.decorators = CollectDecorators(trigger_node);
  return overview;
}

void AppendTriggerPeriodText(const TriggerOverview &trigger,
                             examples::ExampleContext &context,
                             std::ostringstream &line) {
  if (trigger.has_period) {
    line << context.DescribeDuration(trigger.period);
    return;
  }
  if (!trigger.period_text.empty()) {
    line << trigger.period_text;
    return;
  }
  line << "(unspecified period)";
}

void AppendDecorators(const TriggerOverview &trigger,
                      std::ostringstream &line) {
  if (trigger.decorators.empty()) {
    return;
  }
  line << " (decorated by ";
  for (std::size_t index = 0; index < trigger.decorators.size(); ++index) {
    if (index > 0) {
      line << ", ";
    }
    line << trigger.decorators[index];
  }
  line << ')';
}

void LogTriggerLine(const TriggerOverview &trigger,
                    examples::ExampleContext &context) {
  std::ostringstream line;
  line << "  - '" << trigger.name << "' fires every ";
  AppendTriggerPeriodText(trigger, context, line);
  line << " and executes " << trigger.action_summary;
  AppendDecorators(trigger, line);
  context.Log(line.str());
}

void LogTriggerOverview(const std::vector<TriggerOverview> &triggers,
                        examples::ExampleContext &context) {
  if (triggers.empty()) {
    context.Log("No triggers were scheduled by this configuration.");
    return;
  }
  context.Log("Configured triggers:");
  for (const auto &trigger : triggers) {
    LogTriggerLine(trigger, context);
  }
}

void LogTriggerConfiguration(
    const action_graph::yaml_cpp_configuration::Node &configuration,
    examples::ExampleContext &context) {
  std::vector<TriggerOverview> trigger_overviews;
  for (std::size_t index = 0; index < configuration.Size(); ++index) {
    const auto &entry = configuration.Get(index);
    if (!entry.HasKey("trigger")) {
      continue;
    }
    trigger_overviews.push_back(BuildTriggerOverview(entry, context));
  }
  LogTriggerOverview(trigger_overviews, context);
}

void LogActionConfiguration(
    const action_graph::yaml_cpp_configuration::Node &configuration,
    examples::ExampleContext &context) {
  context.Log("Configured action graph:");
  DescribeActionTree(configuration.Get("action"), context, 2);
}

void LogConfigurationOverview(
    const action_graph::yaml_cpp_configuration::Node &configuration,
    examples::ExampleContext &context) {
  if (configuration.IsSequence()) {
    LogTriggerConfiguration(configuration, context);
    return;
  }
  if (configuration.IsMap() && configuration.HasKey("action")) {
    LogActionConfiguration(configuration, context);
    return;
  }
  context.Log("Configuration did not contain triggers or actions that could be "
              "summarised.");
}

void ReportMissingMessage(const ConfigurationNode &node,
                          const std::string &name,
                          examples::ExampleContext &context) {
  std::ostringstream error;
  error << "Configuration for log_message '" << name
        << "' is missing a message field:\n"
        << node.AsString();
  context.Log(error.str());
}

std::string ExtractMessage(const ConfigurationNode &node,
                           const std::string &name,
                           examples::ExampleContext &context) {
  if (node.HasKey("message")) {
    return node.Get("message").AsString();
  }
  ReportMissingMessage(node, name, context);
  throw action_graph::builder::ConfigurationError(
      "log_message action requires a message.", node);
}

void ExecuteLogMessage(examples::ExampleContext &context,
                       const std::string &name, const std::string &message) {
  const auto offset = context.RecordExecution(name);
  std::ostringstream output;
  output << name << " (" << context.DescribeOffset(offset) << "): " << message;
  context.Log(output.str());
}

action_graph::builder::ActionObject
BuildLogMessageAction(const ConfigurationNode &node,
                      examples::ExampleContext &context) {
  const auto name = node.Get("name").AsString();
  const auto message = ExtractMessage(node, name, context);
  return std::make_unique<action_graph::SingleAction>(
      name, [&context, name, message]() {
        ExecuteLogMessage(context, name, message);
      });
}

std::chrono::nanoseconds ExtractWaitDuration(const ConfigurationNode &node) {
  const auto parsed_duration =
      action_graph::builder::ParseDuration(node.Get("duration").AsString());
  return std::chrono::duration_cast<std::chrono::nanoseconds>(parsed_duration);
}

void DescribeWaitStart(examples::ExampleContext &context,
                       const std::string &name,
                       const std::string &duration_text,
                       examples::SteadyClock::duration offset) {
  std::ostringstream start_message;
  start_message << name << " (" << context.DescribeOffset(offset)
                << "): simulating work for " << duration_text;
  context.Log(start_message.str());
}

void DescribeWaitFinish(examples::ExampleContext &context,
                        const std::string &name,
                        examples::SteadyClock::duration elapsed) {
  std::ostringstream finish_message;
  finish_message << name << ": finished simulated work in "
                 << context.DescribeDuration(elapsed);
  context.Log(finish_message.str());
}

void ExecuteWaitAction(examples::ExampleContext &context,
                       const std::string &name,
                       const std::string &duration_text,
                       std::chrono::nanoseconds wait_duration) {
  const auto offset = context.RecordExecution(name);
  DescribeWaitStart(context, name, duration_text, offset);
  const auto start_time = examples::SteadyClock::now();
  std::this_thread::sleep_for(wait_duration);
  const auto elapsed = examples::SteadyClock::now() - start_time;
  DescribeWaitFinish(context, name, elapsed);
}

action_graph::builder::ActionObject
BuildWaitAction(const ConfigurationNode &node,
                examples::ExampleContext &context) {
  const auto name = node.Get("name").AsString();
  const auto duration_text = node.Get("duration").AsString();
  const auto wait_duration = ExtractWaitDuration(node);
  return std::make_unique<action_graph::SingleAction>(
      name, [&context, name, duration_text, wait_duration]() {
        ExecuteWaitAction(context, name, duration_text, wait_duration);
      });
}

action_graph::builder::ActionObject
DecorateTimingMonitor(const ConfigurationNode &node,
                      examples::ExampleContext &context,
                      action_graph::builder::ActionObject action) {
  using action_graph::builder::DecorateWithTimingMonitor;
  std::ostringstream details;
  details << "Attaching timing monitor to '" << action->name
          << "' with a duration limit of "
          << node.Get("duration_limit").AsString()
          << " and an expected period of "
          << node.Get("expected_period").AsString();
  context.Log(details.str());
  return DecorateWithTimingMonitor<examples::SteadyClock>(
      node, std::move(action), context);
}

action_graph::builder::ActionObject BuildTriggerAction(
    const ConfigurationNode &trigger,
    const action_graph::builder::GenericActionBuilder &builder,
    const action_graph::builder::GenericActionDecorator &decorator) {
  auto action = builder(trigger);
  action = decorator(trigger, std::move(action));
  if (!trigger.HasKey("action")) {
    return action;
  }
  const auto &inner_action = trigger.Get("action");
  return decorator(inner_action, std::move(action));
}

examples::SteadyClock::duration
ConvertTriggerPeriod(const std::string &period_text) {
  const auto parsed = action_graph::builder::ParseDuration(period_text);
  return std::chrono::duration_cast<examples::SteadyClock::duration>(parsed);
}

void ScheduleAction(examples::Timer &timer,
                    examples::SteadyClock::duration period,
                    action_graph::builder::ActionObject &action) {
  auto *action_ptr = action.get();
  timer.SetTriggerTime(period, [action_ptr]() { action_ptr->Execute(); });
}

void LogScheduledTrigger(const std::string &trigger_name,
                         const std::string &period_text,
                         examples::ExampleContext &context) {
  std::ostringstream overview;
  overview << "Scheduled trigger '" << trigger_name << "' every " << period_text
           << '.';
  context.Log(overview.str());
}

void ValidateTriggerEntry(const ConfigurationNode &entry) {
  using action_graph::builder::ConfigurationError;
  if (entry.HasKey("trigger")) {
    return;
  }
  throw ConfigurationError("Only trigger nodes are allowed on top level.",
                           entry);
}

const ConfigurationNode &GetTriggerNode(const ConfigurationNode &entry) {
  return entry.Get("trigger");
}

std::string GetTriggerName(const ConfigurationNode &trigger) {
  return trigger.Get("name").AsString();
}

std::string GetTriggerPeriodText(const ConfigurationNode &trigger) {
  return trigger.Get("period").AsString();
}

action_graph::builder::ActionObject CreateScheduledAction(
    const ConfigurationNode &entry,
    const action_graph::builder::GenericActionBuilder &builder,
    const action_graph::builder::GenericActionDecorator &decorator,
    examples::ExampleContext &context, examples::Timer &timer) {
  ValidateTriggerEntry(entry);
  const auto &trigger = GetTriggerNode(entry);
  const auto trigger_name = GetTriggerName(trigger);
  const auto period_text = GetTriggerPeriodText(trigger);
  const auto period = ConvertTriggerPeriod(period_text);
  auto action = BuildTriggerAction(trigger, builder, decorator);
  ScheduleAction(timer, period, action);
  LogScheduledTrigger(trigger_name, period_text, context);
  return action;
}

} // namespace

namespace examples {

double ToMilliseconds(SteadyClock::duration duration) {
  return std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(
             duration)
      .count();
}

double ToMicroseconds(SteadyClock::duration duration) {
  return std::chrono::duration_cast<std::chrono::duration<double, std::micro>>(
             duration)
      .count();
}

bool UseSecondsFormat(double milliseconds) { return milliseconds >= 1000.0; }

bool UseMicrosecondsFormat(double milliseconds) { return milliseconds < 1.0; }

std::string FormatDurationValue(double value, const std::string &unit) {
  std::ostringstream stream;
  stream << std::fixed << std::setprecision(2) << value << unit;
  return stream.str();
}

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

void ExampleContext::UpdateStats(const std::string &action_name,
                                 SteadyClock::duration offset) {
  auto &stats = action_stats_[action_name];
  ++stats.count;
  stats.offsets.push_back(offset);
  if (stats.count == 1) {
    action_order_.push_back(action_name);
  }
}

SteadyClock::duration
ExampleContext::RecordExecution(const std::string &action_name) {
  const auto now = SteadyClock::now();
  const auto offset = now - start_time_;
  std::lock_guard<std::mutex> lock(mutex_);
  UpdateStats(action_name, offset);
  return offset;
}

std::string ExampleContext::DescribeOffset(SteadyClock::duration offset) const {
  return std::string{"+"} + FormatDuration(offset);
}

std::string
ExampleContext::DescribeDuration(SteadyClock::duration duration) const {
  return FormatDuration(duration);
}

void ExampleContext::AppendIntervalDetails(const ActionStats &stats,
                                           std::ostringstream &line) const {
  if (stats.offsets.size() <= 1) {
    return;
  }
  SteadyClock::duration total_interval{};
  for (std::size_t i = 1; i < stats.offsets.size(); ++i) {
    total_interval += stats.offsets[i] - stats.offsets[i - 1];
  }
  const auto average_interval =
      total_interval /
      static_cast<SteadyClock::duration::rep>(stats.offsets.size() - 1);
  line << ", avg interval " << DescribeDuration(average_interval);
}

void ExampleContext::AppendActionSummary(
    const std::string &name, const ActionStats &stats,
    std::vector<std::string> &lines) const {
  std::ostringstream line;
  line << "  - " << name << ": executed " << stats.count << " time"
       << (stats.count == 1 ? "" : "s");
  if (!stats.offsets.empty()) {
    line << ", first at " << DescribeOffset(stats.offsets.front());
    line << ", last at " << DescribeOffset(stats.offsets.back());
  }
  AppendIntervalDetails(stats, line);
  lines.push_back(line.str());
}

void ExampleContext::AppendActionSummaries(
    std::vector<std::string> &lines) const {
  for (const auto &name : action_order_) {
    const auto iterator = action_stats_.find(name);
    if (iterator != action_stats_.end()) {
      AppendActionSummary(name, iterator->second, lines);
    }
  }
}

std::vector<std::string>
ExampleContext::BuildSummaryLines(const std::string &example_title) {
  std::lock_guard<std::mutex> lock(mutex_);
  std::vector<std::string> lines;
  lines.emplace_back("\n--- Summary for " + example_title + " ---");
  if (action_order_.empty()) {
    lines.emplace_back("  (No actions were executed.)");
    return lines;
  }
  AppendActionSummaries(lines);
  return lines;
}

void ExampleContext::LogSummaryLines(const std::vector<std::string> &lines) {
  for (const auto &line : lines) {
    out_ << line << std::endl;
  }
}

void ExampleContext::PrintSummary(const std::string &example_title) {
  const auto lines = BuildSummaryLines(example_title);
  LogSummaryLines(lines);
}

std::string
ExampleContext::FormatDuration(SteadyClock::duration duration) const {
  const auto milliseconds = ToMilliseconds(duration);
  if (UseSecondsFormat(milliseconds)) {
    return FormatDurationValue(milliseconds / 1000.0, " s");
  }
  if (UseMicrosecondsFormat(milliseconds)) {
    return FormatDurationValue(ToMicroseconds(duration), " µs");
  }
  return FormatDurationValue(milliseconds, " ms");
}

action_graph::builder::GenericActionBuilder
CreateExampleActionBuilder(ExampleContext &context) {
  using action_graph::builder::ActionBuilder;
  auto builder =
      action_graph::builder::CreateGenericActionBuilderWithDefaultActions();
  builder.AddBuilderFunction(
      "log_message",
      [&context](const ConfigurationNode &node, const ActionBuilder &) {
        return BuildLogMessageAction(node, context);
      });
  builder.AddBuilderFunction(
      "wait", [&context](const ConfigurationNode &node, const ActionBuilder &) {
        return BuildWaitAction(node, context);
      });
  return builder;
}

action_graph::builder::GenericActionDecorator
CreateExampleActionDecorator(ExampleContext &context) {
  action_graph::builder::GenericActionDecorator decorator;
  decorator.AddDecoratorFunction(
      "timing_monitor", [&context](const ConfigurationNode &node,
                                   action_graph::builder::ActionObject action) {
        return DecorateTimingMonitor(node, context, std::move(action));
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
  std::vector<action_graph::builder::ActionObject> actions;
  for (std::size_t entry_index = 0; entry_index < configuration.Size();
       ++entry_index) {
    const auto &entry = configuration.Get(entry_index);
    actions.push_back(
        CreateScheduledAction(entry, builder, decorator, context, timer));
  }
  return actions;
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
  LogSessionHeader();
  LogConfigurationYaml();
  LogSessionOverview();
}

ExampleSession::~ExampleSession() { context_.PrintSummary(title_); }

void ExampleSession::LogSessionHeader() {
  context_.Log(std::string{"\n=== "} + title_ + " ===");
  context_.Log("Configuration YAML:");
}

void ExampleSession::LogConfigurationYaml() {
  std::istringstream configuration_stream(configuration_yaml_);
  std::string line;
  while (std::getline(configuration_stream, line)) {
    context_.Log("  " + line);
  }
  context_.Log("");
}

void ExampleSession::LogSessionOverview() {
  LogConfigurationOverview(configuration_, context_);
}

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
