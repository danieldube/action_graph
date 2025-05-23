#include <action_graph/action_sequence.h>
#include <action_graph/single_action.h>
#include <algorithm>
#include <gtest/gtest.h>

using action_graph::Action;
using action_graph::ActionSequence;
using action_graph::SingleAction;

TEST(ActionSequencePerformance, many_actions) {
  constexpr std::size_t kActionsCount = 10000;

  std::vector<std::unique_ptr<Action>> actions;
  actions.reserve(kActionsCount);

  std::size_t counter = 0;
  std::generate_n(std::back_inserter(actions), kActionsCount, [&counter]() {
    return std::make_unique<SingleAction>("action",
                                          [&counter]() { ++counter; });
  });
  action_graph::ActionSequence sequence("sequence", std::move(actions));

  sequence.Execute();

  EXPECT_EQ(counter, kActionsCount);
}

TEST(ActionSequencePerformance, many_iterations) {
  constexpr std::size_t kIterationCount = 100000;
  std::size_t counter = 0;

  action_graph::ActionSequence sequence(
      "sequence",
      std::make_unique<SingleAction>("action", [&counter]() { ++counter; }));

  for (size_t iteration = 0; iteration < kIterationCount; ++iteration) {
    sequence.Execute();
  }
  EXPECT_EQ(counter, kIterationCount);
}
