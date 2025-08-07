// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#include <gtest/gtest.h>

#include <native_configuration/map_node.h>
#include <native_configuration/scalar_node.h>
#include <native_configuration/sequence_node.h>

using namespace action_graph::native_configuration;
using action_graph::builder::ConfigurationNode;

class ScalarNodeFixture : public ::testing::Test {
public:
  ScalarNodeFixture() : node{"value"} {}

protected:
  ScalarNode node;
};

TEST_F(ScalarNodeFixture, is_methods) {
  EXPECT_TRUE(node.IsScalar());
  EXPECT_FALSE(node.IsMap());
  EXPECT_FALSE(node.IsSequence());
}

TEST_F(ScalarNodeFixture, HasKey) { EXPECT_FALSE(node.HasKey("any_key")); }

TEST_F(ScalarNodeFixture, get_by_key) {
  using NotFound = ConfigurationNode::ConfigurationNodeNotFound;
  EXPECT_THROW(node.Get("any_key"), NotFound);
}

TEST_F(ScalarNodeFixture, get_by_index) {
  using NotFound = ConfigurationNode::ConfigurationNodeNotFound;
  EXPECT_THROW(node.Get(0), NotFound);
}

TEST_F(ScalarNodeFixture, Size) { EXPECT_EQ(node.Size(), 0); }

TEST_F(ScalarNodeFixture, AsString) { EXPECT_EQ(node.AsString(), "value"); }
