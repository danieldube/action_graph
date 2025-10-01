// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.
#ifndef TESTS_ACTION_GRAPH_TEST_CLOCK_H_
#define TESTS_ACTION_GRAPH_TEST_CLOCK_H_

#include <atomic>
#include <chrono>
#include <cstdint>

// NOLINTBEGIN(*identifier-naming)
class TestClock {
public:
  using rep = int64_t;
  using period = std::milli;
  using duration = std::chrono::duration<rep, period>;
  using time_point = std::chrono::time_point<TestClock, duration>;

  static constexpr bool is_steady = true;

  static time_point now() noexcept {
    return time_point(duration{current_time_.load()});
  }

  static void advance_time(const duration &delta) noexcept {
    current_time_.fetch_add(delta.count());
  }

  static void reset() noexcept { current_time_.store(0); }

private:
  static std::atomic<rep> current_time_;
};
// NOLINTEND(*identifier-naming)

#endif // TESTS_ACTION_GRAPH_TEST_CLOCK_H_
