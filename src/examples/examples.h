// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#ifndef ACTION_GRAPH_EXAMPLES_EXAMPLES_H
#define ACTION_GRAPH_EXAMPLES_EXAMPLES_H

#include "console_log.h"

namespace action_graph_examples {

void RunSingleSecondTriggerExample(ConsoleLog &log);
void RunHighFrequencyTriggerExample(ConsoleLog &log);
void RunCompositeGraphOnce(ConsoleLog &log);
void RunMonitoredGraph(ConsoleLog &log);

} // namespace action_graph_examples

#endif // ACTION_GRAPH_EXAMPLES_EXAMPLES_H
