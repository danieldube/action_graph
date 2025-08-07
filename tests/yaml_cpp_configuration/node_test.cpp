// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#include <gtest/gtest.h>

#include <action_graph/builder/configuration_node.h>
#include <yaml_cpp_configuration/yaml_node.h>

using action_graph::yaml_cpp_configuration::Node;
using namespace action_graph::builder;

const std::string kYamlMap = R"(
key1: value1
key2: value2
)";

const std::string kYamlSequence = R"(
- item1
- item2
)";

const std::string kYamlScalar = R"(
"This is a scalar value"
)";

class NodeTest : public ::testing::Test {
protected:
protected:
  Node map = Node::CreateFromString(kYamlMap);
  Node sequence = Node::CreateFromString(kYamlSequence);
  Node scalar = Node::CreateFromString(kYamlScalar);
};

TEST_F(NodeTest, IsMap) {
  EXPECT_TRUE(map.IsMap());
  EXPECT_FALSE(sequence.IsMap());
  EXPECT_FALSE(scalar.IsMap());
}

TEST_F(NodeTest, IsSequence) {
  EXPECT_FALSE(map.IsSequence());
  EXPECT_TRUE(sequence.IsSequence());
  EXPECT_FALSE(scalar.IsSequence());
}

TEST_F(NodeTest, IsScalar) {
  EXPECT_FALSE(map.IsScalar());
  EXPECT_FALSE(sequence.IsScalar());
  EXPECT_TRUE(scalar.IsScalar());
}

TEST_F(NodeTest, HasKey) {
  EXPECT_TRUE(map.HasKey("key1"));
  EXPECT_TRUE(map.HasKey("key2"));
  EXPECT_FALSE(map.HasKey("key3"));
  EXPECT_FALSE(sequence.HasKey("item1"));
  EXPECT_FALSE(scalar.HasKey("scalar_key"));
}

TEST_F(NodeTest, GetMapValue) {
  EXPECT_EQ(map.Get("key1").AsString(), "value1");
  EXPECT_EQ(map.Get("key2").AsString(), "value2");
  EXPECT_THROW(map.Get("key3"), ConfigurationNode::ConfigurationNodeNotFound);
  EXPECT_THROW(sequence.Get("item1"),
               ConfigurationNode::ConfigurationNodeNotFound);
  EXPECT_THROW(scalar.Get("scalar_key"),
               ConfigurationNode::ConfigurationNodeNotFound);
}

TEST_F(NodeTest, GetSequenceValue) {
  EXPECT_EQ(sequence.Get(0).AsString(), "item1");
  EXPECT_EQ(sequence.Get(1).AsString(), "item2");
  EXPECT_THROW(sequence.Get(2), ConfigurationNode::ConfigurationNodeNotFound);
  EXPECT_THROW(map.Get(0), ConfigurationNode::ConfigurationNodeNotFound);
  EXPECT_THROW(scalar.Get(0), ConfigurationNode::ConfigurationNodeNotFound);
}

TEST_F(NodeTest, Size) {
  EXPECT_EQ(map.Size(), 0);
  EXPECT_EQ(sequence.Size(), 2);
  EXPECT_EQ(scalar.Size(), 0);
}

TEST_F(NodeTest, AsString) {
  EXPECT_EQ(map.AsString(), "key1: value1\nkey2: value2");
  EXPECT_EQ(sequence.AsString(), "- item1\n- item2");
  EXPECT_EQ(scalar.AsString(), "This is a scalar value");
}
