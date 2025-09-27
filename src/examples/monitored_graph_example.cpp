#include "examples.h"

#include "logging_builder.h"
#include "timer_helpers.h"

#include <action_graph/builder/builder.h>
#include <action_graph/builder/generic_action_decorator.h>
#include <yaml_cpp_configuration/yaml_node.h>

#include <chrono>

namespace action_graph_examples {

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

  action_graph::builder::GenericActionDecorator decorator{};
  decorator.AddDecoratorFunction(
      "timing_monitor",
      [&log](const action_graph::builder::ConfigurationNode &node,
             action_graph::builder::ActionObject action) {
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
