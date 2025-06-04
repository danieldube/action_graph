#include <action_graph/builder/parse_duration.h>
#include <gtest/gtest.h>

TEST(ParseDuration, seconds) {
  EXPECT_EQ(action_graph::builder::ParseDuration("10 seconds"),
            std::chrono::seconds(10));
}

TEST(ParseDuration, milliseconds) {
  EXPECT_EQ(action_graph::builder::ParseDuration("10 milliseconds"),
            std::chrono::milliseconds(10));
}

TEST(ParseDuration, microseconds) {
  EXPECT_EQ(action_graph::builder::ParseDuration("10 microseconds"),
            std::chrono::microseconds(10));
}

TEST(ParseDuration, nanoseconds) {
  EXPECT_EQ(action_graph::builder::ParseDuration("10 nanoseconds"),
            std::chrono::nanoseconds(10));
}
