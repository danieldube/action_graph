// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#ifndef ACTION_GRAPH_TESTS_ACTION_GRAPH_TEST_CLOCK_H_
#define ACTION_GRAPH_TESTS_ACTION_GRAPH_TEST_CLOCK_H_

#include <atomic>
#include <chrono>

// NOLINTBEGIN(*identifier-naming)
class TestClock {
public:
  using rep = int64_t;
  using period = std::milli;
  using duration = std::chrono::duration<rep, period>;
  using time_point = std::chrono::time_point<TestClock, duration>;

  static constexpr bool is_steady = true;

  static time_point now() noexcept { return time_point(current_time_.load()); }

  static void advance_time(const duration &delta) noexcept {
    current_time_ = current_time_.load() + delta;
  }

  static void reset() noexcept { current_time_ = duration{0}; }

private:
  static std::atomic<duration> current_time_;
};
// NOLINTEND(*identifier-naming)

#endif // ACTION_GRAPH_TESTS_ACTION_GRAPH_TEST_CLOCK_H_
