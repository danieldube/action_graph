#include "examples.h"

#include "logging_builder.h"
#include "timer_helpers.h"

#include <action_graph/builder/builder.h>
#include <yaml_cpp_configuration/yaml_node.h>

#include <chrono>
#include <memory>

namespace action_graph_examples {

using action_graph::builder::BuildActionGraph;
using action_graph::yaml_cpp_configuration::Node;
using action_graph::GlobalTimer;

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

  auto configuration = Node::CreateFromString(kYaml);
  auto builder = CreateLoggingActionBuilder(log);
  auto timer = std::make_unique<GlobalTimer<TimerClock>>();
  const auto actions = BuildActionGraph(configuration, builder, *timer);
  log.LogMessage("Registered " + std::to_string(actions.size()) +
                 " high-frequency actions.");
  RunTimerFor(*timer, std::chrono::milliseconds(60));
  timer.reset();
}

} // namespace action_graph_examples
