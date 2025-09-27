#include "examples.h"

#include "logging_builder.h"

#include <action_graph/builder/builder.h>
#include <yaml_cpp_configuration/yaml_node.h>

namespace action_graph_examples {

using action_graph::yaml_cpp_configuration::Node;

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

  auto configuration = Node::CreateFromString(kYaml);
  auto builder = CreateLoggingActionBuilder(log);
  auto action = builder(configuration);
  log.LogMessage("Executing composite graph once.");
  action->Execute();
}

} // namespace action_graph_examples
