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

class MapNodeFixture : public ::testing::Test {
public:
  MapNodeFixture()
      : node{std::make_pair("key1", ScalarNode("value1")),
             std::make_pair("key2", SequenceNode()),
             std::make_pair("key3", MapNode())} {}

protected:
  MapNode node;
};

TEST_F(MapNodeFixture, is_methods) {
  EXPECT_FALSE(node.IsScalar());
  EXPECT_TRUE(node.IsMap());
  EXPECT_FALSE(node.IsSequence());
}

TEST_F(MapNodeFixture, HasKey) {
  EXPECT_TRUE(node.HasKey("key1"));
  EXPECT_FALSE(node.HasKey("non_existent_key"));
}

TEST_F(MapNodeFixture, get_by_key) {
  EXPECT_EQ(node.Get("key1").AsString(), "value1");
  EXPECT_THROW(node.Get("non_existent_key"),
               ConfigurationNode::ConfigurationNodeNotFound);
}

TEST_F(MapNodeFixture, get_by_index) {
  using NotFound = ConfigurationNode::ConfigurationNodeNotFound;
  EXPECT_EQ(node.Get(0).AsString(), "value1");
  EXPECT_THROW(node.Get(3), NotFound);
}

TEST_F(MapNodeFixture, Size) { EXPECT_EQ(node.Size(), 3); }

TEST_F(MapNodeFixture, AsString) {
  EXPECT_EQ(node.AsString(), "key1: value1\n"
                             "key2: \n"
                             "key3: \n");
}
