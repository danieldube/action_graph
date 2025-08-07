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

class SequenceNodeFixture : public ::testing::Test {
public:
  SequenceNodeFixture()
      : node{ScalarNode("value1"), SequenceNode(), MapNode()} {}

protected:
  SequenceNode node;
};

TEST_F(SequenceNodeFixture, is_methods) {
  EXPECT_FALSE(node.IsScalar());
  EXPECT_FALSE(node.IsMap());
  EXPECT_TRUE(node.IsSequence());
}

TEST_F(SequenceNodeFixture, HasKey) { EXPECT_FALSE(node.HasKey("any_key")); }

TEST_F(SequenceNodeFixture, get_by_key) {
  using NotFound = ConfigurationNode::ConfigurationNodeNotFound;
  EXPECT_THROW(node.Get("any_key"), NotFound);
}

TEST_F(SequenceNodeFixture, get_by_index) {
  EXPECT_EQ(node.Get(0).AsString(), "value1");
  EXPECT_THROW(node.Get(3), ConfigurationNode::ConfigurationNodeNotFound);
}

TEST_F(SequenceNodeFixture, Size) { EXPECT_EQ(node.Size(), 3); }

TEST_F(SequenceNodeFixture, AsString) {
  EXPECT_EQ(node.AsString(), "- value1\n"
                             "- \n"
                             "- \n");
}
