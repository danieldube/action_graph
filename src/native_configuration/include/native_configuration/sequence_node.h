// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#ifndef ACTION_GRAPH_SRC_NATIVE_CONFIGURATION_INCLUDE_NATIVE_CONFIGURATION_SEQUENCE_NODE_H_
#define ACTION_GRAPH_SRC_NATIVE_CONFIGURATION_INCLUDE_NATIVE_CONFIGURATION_SEQUENCE_NODE_H_

#include <action_graph/builder/configuration_node.h>
#include <memory>
#include <sstream>
#include <type_traits>
#include <utility>
#include <vector>

namespace action_graph {
namespace native_configuration {
class SequenceNode : public builder::ConfigurationNode {
public:
  using Entry = ConfigurationNode::Pointer;

  explicit SequenceNode(std::vector<ConfigurationNode::Pointer> entries)
      : entries_(std::move(entries)) {}

  template <typename... Nodes> explicit SequenceNode(Nodes... nodes) {
    entries_.reserve(sizeof...(nodes));
    AddEntries(std::move(nodes)...);
  }

  bool IsScalar() const noexcept override { return false; }
  bool IsMap() const noexcept override { return false; }
  bool IsSequence() const noexcept override { return true; }

  bool HasKey(const std::string &key) const noexcept override { return false; }

  Reference Get(const std::string &key) const override {
    throw ConfigurationNodeNotFound("Sequence Node don't support index access.",
                                    *this);
  }

  Reference Get(std::size_t index) const override {
    if (index < entries_.size()) {
      return *entries_[index];
    }
    throw ConfigurationNodeNotFound("Index is out of range.", *this);
  }
  std::size_t Size() const noexcept override { return entries_.size(); }
  std::string AsString() const noexcept override {
    std::stringstream stream;
    for (const auto &entry : entries_)
      stream << "- " << entry->AsString() << std::endl;
    return stream.str();
  }

private:
  void AddEntries() {}

  template <typename Node, typename... Remaining>
  void AddEntries(Node node, Remaining... remaining) {
    using NodeType = typename std::decay<Node>::type;
    entries_.emplace_back(
        std::make_unique<NodeType>(std::move(node)));
    AddEntries(std::move(remaining)...);
  }

  std::vector<ConfigurationNode::Pointer> entries_;
};

} // namespace native_configuration
} // namespace action_graph

#endif // ACTION_GRAPH_SRC_NATIVE_CONFIGURATION_INCLUDE_NATIVE_CONFIGURATION_SEQUENCE_NODE_H_
