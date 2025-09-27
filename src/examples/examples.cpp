// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#include <action_graph/builder/builder.h>
#include <action_graph/builder/generic_action_builder.h>
#include <action_graph/builder/generic_action_decorator.h>
#include <action_graph/builder/parse_duration.h>
#include <action_graph/global_timer/global_timer.h>
#include <action_graph/log.h>
#include <action_graph/single_action.h>
#include <yaml_cpp_configuration/yaml_node.h>

#include <chrono>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace action_graph_examples {

using TimerClock = std::chrono::steady_clock;
using action_graph::builder::ActionBuilder;
using action_graph::builder::ActionObject;
using action_graph::builder::ConfigurationNode;
using action_graph::builder::GenericActionBuilder;
using action_graph::builder::GenericActionDecorator;

class ConsoleLog final : public action_graph::Log {
public:
  explicit ConsoleLog(std::ostream &output) : output_(output) {}

  void LogMessage(const std::string &message) override {
    WriteLine("[info] " + message);
  }

  void LogError(const std::string &message) override {
    WriteLine("[error] " + message);
  }

private:
  void WriteLine(const std::string &line) {
    std::lock_guard<std::mutex> guard(mutex_);
    output_ << line << std::endl;
  }

  std::ostream &output_;
  std::mutex mutex_{};
};

GenericActionBuilder
CreateLoggingActionBuilder(ConsoleLog &log, GenericActionDecorator decorator) {
  auto builder = action_graph::builder::CreateGenericActionBuilderWithDefaultActions();
  builder.SetActionDecorator(std::move(decorator));
  builder.AddBuilderFunction(
      "log_action",
      [&log](const ConfigurationNode &node, const ActionBuilder &) {
        const auto name = node.Get("name").AsString();
        const auto message = node.Get("message").AsString();

        auto delay = std::chrono::milliseconds::zero();
        if (node.HasKey("delay")) {
          const auto delay_text = node.Get("delay").AsString();
          const auto parsed_delay =
              action_graph::builder::ParseDuration(delay_text);
          delay = std::chrono::duration_cast<std::chrono::milliseconds>(
              parsed_delay);
        }

        return std::make_unique<action_graph::SingleAction>(
            name, [name, message, delay, &log]() {
              if (delay > std::chrono::milliseconds::zero()) {
                std::this_thread::sleep_for(delay);
              }
              log.LogMessage(name + ": " + message);
            });
      });
  return builder;
}

GenericActionBuilder CreateLoggingActionBuilder(ConsoleLog &log) {
  return CreateLoggingActionBuilder(log, GenericActionDecorator{});
}

template <typename Duration>
void RunTimerFor(action_graph::GlobalTimer<TimerClock> &timer,
                 Duration duration) {
  std::this_thread::sleep_for(duration);
  timer.WaitOneCycle();
}

void RunSingleSecondTriggerExample(ConsoleLog &log) {
  log.LogMessage(
      "=== Example: one action triggered every second ===");
  const std::string kYaml = R"(
- trigger:
    name: announce_tick
    period: 1 seconds
    action:
      name: log_tick
      type: log_action
      message: "tick"
)";

  auto configuration =
      action_graph::yaml_cpp_configuration::Node::CreateFromString(kYaml);
  auto builder = CreateLoggingActionBuilder(log);
  action_graph::GlobalTimer<TimerClock> timer{};
  const auto actions =
      action_graph::builder::BuildActionGraph(configuration, builder, timer);
  log.LogMessage("Registered " + std::to_string(actions.size()) +
                 " periodic action.");
  RunTimerFor(timer, std::chrono::seconds(3));
}

void RunHighFrequencyTriggerExample(ConsoleLog &log) {
  log.LogMessage(
      "=== Example: three actions triggered every 10 milliseconds ===");
  const std::string kYaml = R"(
- trigger:
    name: sensor_one
    period: 10 milliseconds
    action:
      name: capture_sensor_one
      type: log_action
      message: "sensor one sampled"
- trigger:
    name: sensor_two
    period: 10 milliseconds
    action:
      name: capture_sensor_two
      type: log_action
      message: "sensor two sampled"
- trigger:
    name: sensor_three
    period: 10 milliseconds
    action:
      name: capture_sensor_three
      type: log_action
      message: "sensor three sampled"
)";

  auto configuration =
      action_graph::yaml_cpp_configuration::Node::CreateFromString(kYaml);
  auto builder = CreateLoggingActionBuilder(log);
  action_graph::GlobalTimer<TimerClock> timer{};
  const auto actions =
      action_graph::builder::BuildActionGraph(configuration, builder, timer);
  log.LogMessage("Registered " + std::to_string(actions.size()) +
                 " high-frequency actions.");
  RunTimerFor(timer, std::chrono::milliseconds(60));
}

void RunCompositeGraphOnce(ConsoleLog &log) {
  log.LogMessage(
      "=== Example: parallel and sequential graph executed once ===");
  const std::string kYaml = R"(
action:
  name: content_pipeline
  type: sequential_actions
  actions:
    - action:
        name: prepare_context
        type: log_action
        message: "prepare context"
    - action:
        name: load_and_process
        type: parallel_actions
        actions:
          - action:
              name: load_assets
              type: log_action
              message: "load assets"
          - action:
              name: process_assets
              type: sequential_actions
              actions:
                - action:
                    name: decode
                    type: log_action
                    message: "decode"
                - action:
                    name: enrich
                    type: log_action
                    message: "enrich"
    - action:
        name: publish
        type: log_action
        message: "publish"
)";

  auto configuration =
      action_graph::yaml_cpp_configuration::Node::CreateFromString(kYaml);
  auto builder = CreateLoggingActionBuilder(log);
  auto action = builder(configuration);
  log.LogMessage("Executing composite graph once.");
  action->Execute();
}

void RunMonitoredGraph(ConsoleLog &log) {
  log.LogMessage(
      "=== Example: timer-driven graph monitored by TimingMonitor ===");
  const std::string kYaml = R"(
- trigger:
    name: monitored_cycle
    period: 200 milliseconds
    action:
      name: monitored_sequence
      type: sequential_actions
      decorate:
        - type: timing_monitor
          duration_limit: 120 milliseconds
          expected_period: 200 milliseconds
      actions:
        - action:
            name: start_cycle
            type: log_action
            message: "start cycle"
        - action:
            name: slow_work
            type: log_action
            message: "simulate load"
            delay: 250 milliseconds
        - action:
            name: finish_cycle
            type: log_action
            message: "finish cycle"
)";

  GenericActionDecorator decorator{};
  decorator.AddDecoratorFunction(
      "timing_monitor",
      [&log](const ConfigurationNode &node, ActionObject action) {
        return action_graph::builder::DecorateWithTimingMonitor<TimerClock>(
            node, std::move(action), log);
      });

  auto configuration =
      action_graph::yaml_cpp_configuration::Node::CreateFromString(kYaml);
  auto builder = CreateLoggingActionBuilder(log, std::move(decorator));
  action_graph::GlobalTimer<TimerClock> timer{};
  const auto actions =
      action_graph::builder::BuildActionGraph(configuration, builder, timer);
  log.LogMessage("Registered " + std::to_string(actions.size()) +
                 " monitored action.");
  RunTimerFor(timer, std::chrono::milliseconds(700));
}

} // namespace action_graph_examples

int main() {
  using namespace action_graph_examples;

  ConsoleLog log{std::cout};
  RunSingleSecondTriggerExample(log);
  RunHighFrequencyTriggerExample(log);
  RunCompositeGraphOnce(log);
  RunMonitoredGraph(log);

  return 0;
}
