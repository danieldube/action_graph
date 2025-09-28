// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#include "console_log.h"
#include "examples.h"

#include <iostream>

int main() {
  using namespace action_graph_examples;

  ConsoleLog log{std::cout};
  RunSingleSecondTriggerExample(log);
  RunHighFrequencyTriggerExample(log);
  RunCompositeGraphOnce(log);
  RunMonitoredGraph(log);

  return 0;
}
