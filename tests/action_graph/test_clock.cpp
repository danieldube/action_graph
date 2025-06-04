#include <test_clock.h>

std::atomic<TestClock::duration> TestClock::current_time_ =
    TestClock::duration{0};
