// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.
#ifndef SRC_EXAMPLES_EXAMPLES_H_
#define SRC_EXAMPLES_EXAMPLES_H_

#include "console_log.h"

namespace action_graph_examples {

void RunSingleSecondTriggerExample(ConsoleLog &log);
void RunHighFrequencyTriggerExample(ConsoleLog &log);
void RunCompositeGraphOnce(ConsoleLog &log);
void RunMonitoredGraph(ConsoleLog &log);

} // namespace action_graph_examples

#endif // SRC_EXAMPLES_EXAMPLES_H_
