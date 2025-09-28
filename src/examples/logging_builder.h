// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#ifndef ACTION_GRAPH_EXAMPLES_LOGGING_BUILDER_H
#define ACTION_GRAPH_EXAMPLES_LOGGING_BUILDER_H

#include <action_graph/builder/generic_action_builder.h>
#include <action_graph/builder/generic_action_decorator.h>

namespace action_graph_examples {

class ConsoleLog;

using action_graph::builder::GenericActionBuilder;
using action_graph::builder::GenericActionDecorator;

GenericActionBuilder CreateLoggingActionBuilder(ConsoleLog &log);

GenericActionBuilder
CreateLoggingActionBuilder(ConsoleLog &log, GenericActionDecorator decorator);

} // namespace action_graph_examples

#endif // ACTION_GRAPH_EXAMPLES_LOGGING_BUILDER_H
