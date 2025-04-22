#include <action_graph/parallel_actions.h>
#include <action_graph/single_action.h>
#include <gtest/gtest.h>

using action_graph::Action;
using action_graph::ParallelActions;
using action_graph::SingleAction;

TEST(ParallelActionsPerformance, many_actions) {
  constexpr std::size_t kActionsCount = 10000;
  std::vector<std::unique_ptr<Action>> actions;
  actions.reserve(kActionsCount);
  std::atomic<std::size_t> counter = 0;
  std::generate_n(std::back_inserter(actions), kActionsCount, [&counter]() {
    return std::make_unique<SingleAction>("action",
                                          [&counter]() { ++counter; });
  });
  action_graph::ParallelActions sequence("sequence", std::move(actions));
  sequence.Execute();
  EXPECT_EQ(counter, kActionsCount);
}

TEST(ParallelActionsPerformance, many_iterations) {
  constexpr std::size_t kActionsCount = 100;
  constexpr std::size_t kIterationCount = 100;

  std::vector<std::unique_ptr<Action>> actions;
  actions.reserve(kActionsCount);

  std::atomic<std::size_t> counter = 0;

  std::generate_n(std::back_inserter(actions), kActionsCount, [&counter]() {
    return std::make_unique<SingleAction>("action",
                                          [&counter]() { ++counter; });
  });
  action_graph::ParallelActions sequence("sequence", std::move(actions));

  for (size_t iteration = 0; iteration < kIterationCount; ++iteration) {
    sequence.Execute();
  }
  EXPECT_EQ(counter, kActionsCount * kIterationCount);
}

TEST(ParallelActionsPerformance, many_actions_many_iterations) {
  constexpr std::size_t kIterationCount = 10000;
  std::atomic<std::size_t> counter = 0;
  action_graph::ParallelActions sequence(
      "sequence",
      std::make_unique<SingleAction>("action", [&counter]() { ++counter; }));
  for (size_t iteration = 0; iteration < kIterationCount; ++iteration) {
    sequence.Execute();
  }
  EXPECT_EQ(counter, kIterationCount);
}
