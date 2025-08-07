// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#ifndef ACTION_GRAPH_SRC_YAML_CPP_CONFIGURATION_INCLUDE_YAML_CPP_CONFIGURATION_YAML_NODE_H_
#define ACTION_GRAPH_SRC_YAML_CPP_CONFIGURATION_INCLUDE_YAML_CPP_CONFIGURATION_YAML_NODE_H_

#include <action_graph/builder/configuration_node.h>
#include <yaml-cpp/yaml.h>

namespace action_graph {
namespace yaml_cpp_configuration {
class Node : public action_graph::builder::ConfigurationNode {
public:
  bool IsScalar() const noexcept override {
    return node_.Type() == YAML::NodeType::Scalar;
  }

  bool IsMap() const noexcept override {
    return node_.Type() == YAML::NodeType::Map;
  }

  bool IsSequence() const noexcept override {
    return node_.Type() == YAML::NodeType::Sequence;
  }

  bool HasKey(const std::string &key) const noexcept override {
    return node_.IsMap() && node_[key];
  }

  Reference Get(const std::string &key) const override {
    if (!node_.IsMap()) {
      throw ConfigurationNodeNotFound("Node is not a map", *this);
    }
    if (!HasKey(key)) {
      throw ConfigurationNodeNotFound("Key not found: " + key, *this);
    }
    auto cached_entry = map_.find(key);
    if (cached_entry == map_.end()) {
      const auto &yaml_node = node_[key];
      cached_entry = map_.emplace(key, Node(yaml_node)).first;
    }
    return cached_entry->second;
  }

  Reference Get(size_t index) const override {
    if (!node_.IsSequence()) {
      throw ConfigurationNodeNotFound("Node is not a sequence", *this);
    }
    if (index >= node_.size()) {
      throw ConfigurationNodeNotFound(
          "Index out of bounds: " + std::to_string(index), *this);
    }
    auto cached_entry = sequence_.find(index);
    if (cached_entry == sequence_.end()) {
      const auto &yaml_node = node_[index];
      cached_entry = sequence_.emplace(index, Node(yaml_node)).first;
    }
    return cached_entry->second;
  }

  std::size_t Size() const noexcept override {
    if (node_.IsSequence()) {
      return node_.size();
    }
    return 0;
  }

  std::string AsString() const noexcept override { return YAML::Dump(node_); }

  static Node CreateFromString(const std::string &yaml_string) {
    YAML::Node node = YAML::Load(yaml_string);
    if (!node) {
      throw std::runtime_error("Failed to load YAML from string.");
    }
    return Node(node);
  }

private:
  explicit Node(const YAML::Node &node) : node_(node) {
    if (!node_) {
      throw std::runtime_error("YAML node is nullptr.");
    }
  }

  YAML::Node node_;
  mutable std::map<std::string, Node> map_{};
  mutable std::map<size_t, Node> sequence_{};
};
} // namespace yaml_cpp_configuration
} // namespace action_graph

#endif // ACTION_GRAPH_SRC_YAML_CPP_CONFIGURATION_INCLUDE_YAML_CPP_CONFIGURATION_YAML_NODE_H_
