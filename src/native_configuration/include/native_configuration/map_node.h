// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#ifndef ACTION_GRAPH_SRC_NATIVE_CONFIGURATION_INCLUDE_NATIVE_CONFIGURATION_MAP_NODE_H_
#define ACTION_GRAPH_SRC_NATIVE_CONFIGURATION_INCLUDE_NATIVE_CONFIGURATION_MAP_NODE_H_

#include <action_graph/builder/configuration_node.h>
#include <cstddef>
#include <iterator>
#include <map>
#include <memory>
#include <sstream>
#include <type_traits>
#include <utility>

namespace action_graph {
namespace native_configuration {
class MapNode final : public builder::ConfigurationNode {
public:
  using Entry = std::pair<std::string, ConfigurationNode::Pointer>;

  explicit MapNode(std::map<std::string, ConfigurationNode::Pointer> entries)
      : entries_(std::move(entries)) {}

  template <typename... Pairs> explicit MapNode(Pairs... pairs) {
    AddEntries(std::move(pairs)...);
  }

  bool IsScalar() const noexcept override { return false; }
  bool IsMap() const noexcept override { return true; }
  bool IsSequence() const noexcept override { return false; }

  bool HasKey(const std::string &key) const noexcept override {
    auto it = entries_.find(key);
    if (it == entries_.end()) {
      return false;
    }
    return true;
  }

  Reference Get(const std::string &key) const override {
    auto it = entries_.find(key);
    if (it != entries_.end()) {
      return *it->second;
    }
    throw ConfigurationNodeNotFound("Key not found: " + key, *this);
  }

  Reference Get(std::size_t index) const override {
    throw ConfigurationNodeNotFound("Map Node does not support index access",
                                    *this);
  }

  std::size_t Size() const noexcept override { return 0; }
  std::string AsString() const noexcept override {
    std::stringstream stream;
    for (const auto &entry : entries_) {
      stream << entry.first << ": " << entry.second->AsString() << std::endl;
    }
    return stream.str();
  }

private:
  static void AddEntries() {}

  template <typename Pair, typename... Remaining>
  void AddEntries(Pair pair, Remaining... remaining) {
    using NodeType = typename std::decay<decltype(pair.second)>::type;
    entries_.emplace(std::move(pair.first),
                     std::make_unique<NodeType>(std::move(pair.second)));
    AddEntries(std::move(remaining)...);
  }

  std::map<std::string, ConfigurationNode::Pointer> entries_;
};

} // namespace native_configuration
} // namespace action_graph

#endif // ACTION_GRAPH_SRC_NATIVE_CONFIGURATION_INCLUDE_NATIVE_CONFIGURATION_MAP_NODE_H_
