// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#include <action_graph/builder/configuration_node.h>
#include <gtest/gtest.h>

using action_graph::builder::ConfigurationNode;

class TestNode : public ConfigurationNode {
public:
  bool IsScalar() const noexcept override { return false; }
  bool IsMap() const noexcept override { return false; }
  bool IsSequence() const noexcept override { return false; }
  bool HasKey(const std::string &) const noexcept override { return false; }
  Reference Get(const std::string &) const override {
    throw std::runtime_error("not implemented");
  }
  Reference Get(std::size_t) const override {
    throw std::runtime_error("not implemented");
  }
  std::size_t Size() const noexcept override { return 0; }
  std::string AsString() const noexcept override { return "TestNode"; }
};

TEST(ConfigurationNodeTest, InterfaceMethods) {
  TestNode node;
  EXPECT_FALSE(node.IsScalar());
  EXPECT_FALSE(node.IsMap());
  EXPECT_FALSE(node.IsSequence());
  EXPECT_EQ(node.Size(), 0u);
  EXPECT_EQ(node.AsString(), "TestNode");
}

TEST(ConfigurationNodeTest, GetThrows) {
  TestNode node;
  EXPECT_THROW(node.Get("key"), std::runtime_error);
  EXPECT_THROW(node.Get(0), std::runtime_error);
}
