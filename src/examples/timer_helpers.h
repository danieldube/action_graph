#ifndef ACTION_GRAPH_EXAMPLES_TIMER_HELPERS_H
#define ACTION_GRAPH_EXAMPLES_TIMER_HELPERS_H

#include <action_graph/global_timer/global_timer.h>

#include <chrono>
#include <thread>

namespace action_graph_examples {

using TimerClock = std::chrono::steady_clock;

template <typename Duration>
void RunTimerFor(action_graph::GlobalTimer<TimerClock> &timer,
                 Duration duration) {
  std::this_thread::sleep_for(duration);
  timer.WaitOneCycle();
}

} // namespace action_graph_examples

#endif // ACTION_GRAPH_EXAMPLES_TIMER_HELPERS_H
