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
