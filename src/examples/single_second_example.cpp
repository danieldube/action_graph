#include "examples.h"

#include "logging_builder.h"
#include "timer_helpers.h"

#include <action_graph/builder/builder.h>
#include <yaml_cpp_configuration/yaml_node.h>

#include <chrono>

namespace action_graph_examples {

void RunSingleSecondTriggerExample(ConsoleLog &log) {
  log.LogMessage("=== Example: one action triggered every second ===");
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

} // namespace action_graph_examples
