// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#include <yaml_cpp_configuration/yaml_node.h>

namespace action_graph {
namespace yaml_cpp_configuration {

bool Node::IsScalar() const noexcept {
  return node_.Type() == YAML::NodeType::Scalar;
}

bool Node::IsMap() const noexcept {
  return node_.Type() == YAML::NodeType::Map;
}

bool Node::IsSequence() const noexcept {
  return node_.Type() == YAML::NodeType::Sequence;
}

bool Node::HasKey(const std::string &key) const noexcept {
  return node_.IsMap() && node_[key];
}

Node::Reference Node::Get(const std::string &key) const {
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

Node::Reference Node::Get(size_t index) const {
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

std::size_t Node::Size() const noexcept {
  if (node_.IsSequence()) {
    return node_.size();
  }
  return 0;
}

std::string Node::AsString() const noexcept { return YAML::Dump(node_); }

Node Node::CreateFromString(const std::string &yaml_string) {
  YAML::Node node = YAML::Load(yaml_string);
  if (!node) {
    throw std::runtime_error("Failed to load YAML from string.");
  }
  return Node(node);
}

Node::Node(const YAML::Node &node) : node_(node) {
  if (!node_) {
    throw std::runtime_error("YAML node is nullptr.");
  }
}

} // namespace yaml_cpp_configuration
} // namespace action_graph
