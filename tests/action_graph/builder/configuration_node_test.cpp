#include <action_graph/builder/configuration_node.h>
#include <gtest/gtest.h>
#include <vector>

using action_graph::builder::ConfigurationNode;

class ScalarNode : public ConfigurationNode {
public:
  explicit ScalarNode(std::string value) : value_(std::move(value)) {}
  bool isScalar() const noexcept override { return true; }
  bool isMap() const noexcept override { return false; }
  bool isSequence() const noexcept override { return false; }

  bool hasKey(const std::string &key) const noexcept override { return false; }
  Reference get(const std::string &key) const override {
    throw ConfigurationNodeNotFound("ScalarNode does not support key access",
                                    *this);
  }
  Reference get(std::size_t index) const override {
    throw ConfigurationNodeNotFound("ScalarNode does not support index access",
                                    *this);
  }
  std::size_t size() const noexcept override { return 0; }
  std::string asString() const noexcept override { return value_; }

private:
  std::string value_;
};

class MapNode : public ConfigurationNode {
public:
  using Entry = std::pair<std::string, ConfigurationNode::Pointer>;

  MapNode(std::map<std::string, ConfigurationNode::Pointer> entries)
      : entries_(std::move(entries)) {}

  template <typename... Pairs> MapNode(Pairs &&...pairs) {
    (entries_.emplace(
         std::forward<Pairs>(pairs).first,
         std::make_unique<
             std::decay_t<decltype(std::forward<Pairs>(pairs).second)>>(
             std::forward<Pairs>(pairs).second)),
     ...);
  }

  bool isScalar() const noexcept override { return false; }
  bool isMap() const noexcept override { return true; }
  bool isSequence() const noexcept override { return false; }

  bool hasKey(const std::string &key) const noexcept override {
    auto it = entries_.find(key);
    if (it == entries_.end()) {
      return false;
    }
    return true;
  }

  Reference get(const std::string &key) const override {
    auto it = entries_.find(key);
    if (it != entries_.end()) {
      return *it->second;
    }
    throw ConfigurationNodeNotFound("Key not found: " + key, *this);
  }

  Reference get(std::size_t index) const override {
    throw ConfigurationNodeNotFound("Map Node does not support index access",
                                    *this);
  }

  std::size_t size() const noexcept override {
    return 0;
  } // Implement size logic
  std::string asString() const noexcept override {
    std::stringstream stream;
    for (const auto &entry : entries_) {
      stream << entry.first << ": " << entry.second->asString() << std::endl;
    }
    return stream.str();
  }

private:
  std::map<std::string, ConfigurationNode::Pointer> entries_;
};

class SequenceNode : public ConfigurationNode {
public:
  using Entry = ConfigurationNode::Pointer;

  explicit SequenceNode(std::vector<ConfigurationNode::Pointer> entries)
      : entries_(std::move(entries)) {}

  template <typename... Nodes> explicit SequenceNode(Nodes &&...nodes) {
    entries_.reserve(sizeof...(nodes));
    (entries_.emplace_back(
         std::make_unique<std::decay_t<Nodes>>(std::forward<Nodes>(nodes))),
     ...);
  }

  bool isScalar() const noexcept override { return false; }
  bool isMap() const noexcept override { return false; }
  bool isSequence() const noexcept override { return true; }

  bool hasKey(const std::string &key) const noexcept override { return false; }

  Reference get(const std::string &key) const override {
    throw ConfigurationNodeNotFound("Sequence Node don't support index access.",
                                    *this);
  }

  Reference get(std::size_t index) const override {
    if (index < entries_.size()) {
      return *entries_[index];
    }
    throw ConfigurationNodeNotFound("Index is out of range.", *this);
  }
  std::size_t size() const noexcept override { return entries_.size(); }
  std::string asString() const noexcept override {
    std::stringstream stream;
    for (const auto &entry : entries_)
      stream << "- " << entry->asString() << std::endl;
    return stream.str();
  }

private:
  std::vector<ConfigurationNode::Pointer> entries_;
};

class ScalarNodeFixture : public ::testing::Test {
public:
  ScalarNodeFixture() : node{"value"} {}

protected:
  ScalarNode node;
};

TEST_F(ScalarNodeFixture, is_methods) {
  EXPECT_TRUE(node.isScalar());
  EXPECT_FALSE(node.isMap());
  EXPECT_FALSE(node.isSequence());
}

TEST_F(ScalarNodeFixture, hasKey) { EXPECT_FALSE(node.hasKey("any_key")); }

TEST_F(ScalarNodeFixture, get_by_key) {
  using NotFound = ConfigurationNode::ConfigurationNodeNotFound;
  EXPECT_THROW(node.get("any_key"), NotFound);
}

TEST_F(ScalarNodeFixture, get_by_index) {
  using NotFound = ConfigurationNode::ConfigurationNodeNotFound;
  EXPECT_THROW(node.get(0), NotFound);
}

TEST_F(ScalarNodeFixture, size) { EXPECT_EQ(node.size(), 0); }

TEST_F(ScalarNodeFixture, asString) { EXPECT_EQ(node.asString(), "value"); }

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
  EXPECT_FALSE(node.isScalar());
  EXPECT_TRUE(node.isMap());
  EXPECT_FALSE(node.isSequence());
}

TEST_F(MapNodeFixture, hasKey) {
  EXPECT_TRUE(node.hasKey("key1"));
  EXPECT_FALSE(node.hasKey("non_existent_key"));
}

TEST_F(MapNodeFixture, get_by_key) {
  EXPECT_EQ(node.get("key1").asString(), "value1");
  EXPECT_THROW(node.get("non_existent_key"),
               ConfigurationNode::ConfigurationNodeNotFound);
}

TEST_F(MapNodeFixture, get_by_index) {
  using NotFound = ConfigurationNode::ConfigurationNodeNotFound;
  EXPECT_THROW(node.get(0), NotFound);
}

TEST_F(MapNodeFixture, size) { EXPECT_EQ(node.size(), 0); }

TEST_F(MapNodeFixture, asString) {
  EXPECT_EQ(node.asString(), "key1: value1\n"
                             "key2: \n"
                             "key3: \n");
}

class SequenceNodeFixture : public ::testing::Test {
public:
  SequenceNodeFixture()
      : node{ScalarNode("value1"), SequenceNode(), MapNode()} {}

protected:
  SequenceNode node;
};

TEST_F(SequenceNodeFixture, is_methods) {
  EXPECT_FALSE(node.isScalar());
  EXPECT_FALSE(node.isMap());
  EXPECT_TRUE(node.isSequence());
}

TEST_F(SequenceNodeFixture, hasKey) { EXPECT_FALSE(node.hasKey("any_key")); }

TEST_F(SequenceNodeFixture, get_by_key) {
  using NotFound = ConfigurationNode::ConfigurationNodeNotFound;
  EXPECT_THROW(node.get("any_key"), NotFound);
}

TEST_F(SequenceNodeFixture, get_by_index) {
  EXPECT_EQ(node.get(0).asString(), "value1");
  EXPECT_THROW(node.get(3), ConfigurationNode::ConfigurationNodeNotFound);
}

TEST_F(SequenceNodeFixture, size) { EXPECT_EQ(node.size(), 3); }

TEST_F(SequenceNodeFixture, asString) {
  EXPECT_EQ(node.asString(), "- value1\n"
                             "- \n"
                             "- \n");
}
