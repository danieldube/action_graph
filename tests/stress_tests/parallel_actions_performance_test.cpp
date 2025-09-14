// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#include <action_graph/parallel_actions.h>
#include <action_graph/single_action.h>
#include <algorithm>
#include <gtest/gtest.h>
#include <thread>

using action_graph::Action;
using action_graph::ParallelActions;
using action_graph::SingleAction;

TEST(ParallelActionsPerformance, many_actions) {
  constexpr std::size_t kActionsCount = 10000;

  std::vector<std::unique_ptr<Action>> actions;
  actions.reserve(kActionsCount);

  std::atomic<size_t> counter = 0;
  std::generate_n(std::back_inserter(actions), kActionsCount, [&counter]() {
    return std::make_unique<SingleAction>("action",
                                          [&counter]() { ++counter; });
  });
  ParallelActions parallel("parallel", std::move(actions));

  parallel.Execute();

  EXPECT_EQ(counter, kActionsCount);
}

TEST(ParallelActionsPerformance, many_threads) {
  constexpr std::size_t kThreadCount = 100;
  std::size_t counter = 0;

  std::vector<std::unique_ptr<Action>> actions;
  actions.reserve(kThreadCount);
  for (size_t i = 0; i < kThreadCount; ++i) {
    actions.push_back(std::make_unique<SingleAction>("action", [&counter]() {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      ++counter;
    }));
  }
  ParallelActions parallel("parallel", std::move(actions));

  parallel.Execute();

  EXPECT_EQ(counter, kThreadCount);
}
