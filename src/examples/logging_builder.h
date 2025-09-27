#ifndef ACTION_GRAPH_EXAMPLES_LOGGING_BUILDER_H
#define ACTION_GRAPH_EXAMPLES_LOGGING_BUILDER_H

#include <action_graph/builder/generic_action_builder.h>
#include <action_graph/builder/generic_action_decorator.h>

namespace action_graph_examples {

class ConsoleLog;

action_graph::builder::GenericActionBuilder
CreateLoggingActionBuilder(ConsoleLog &log);

action_graph::builder::GenericActionBuilder
CreateLoggingActionBuilder(ConsoleLog &log,
                           action_graph::builder::GenericActionDecorator decorator);

} // namespace action_graph_examples

#endif // ACTION_GRAPH_EXAMPLES_LOGGING_BUILDER_H
