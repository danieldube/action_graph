#ifndef ACTION_GRAPH_SRC_NATIVE_CONFIGURATION_INCLUDE_NATIVE_CONFIGURATION_MAP_NODE_H_
#define ACTION_GRAPH_SRC_NATIVE_CONFIGURATION_INCLUDE_NATIVE_CONFIGURATION_MAP_NODE_H_

#include <action_graph/builder/configuration_node.h>
#include <map>
#include <sstream>

namespace action_graph {
namespace native_configuration {
class MapNode : public builder::ConfigurationNode {
public:
  using Entry = std::pair<std::string, ConfigurationNode::Pointer>;

  explicit MapNode(std::map<std::string, ConfigurationNode::Pointer> entries)
      : entries_(std::move(entries)) {}

  template <typename... Pairs> explicit MapNode(Pairs &&...pairs) {
    (entries_.emplace(
         std::forward<Pairs>(pairs).first,
         std::make_unique<
             std::decay_t<decltype(std::forward<Pairs>(pairs).second)>>(
             std::forward<Pairs>(pairs).second)),
     ...);
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

  std::size_t Size() const noexcept override {
    return 0;
  } // Implement size logic
  std::string AsString() const noexcept override {
    std::stringstream stream;
    for (const auto &entry : entries_) {
      stream << entry.first << ": " << entry.second->AsString() << std::endl;
    }
    return stream.str();
  }

private:
  std::map<std::string, ConfigurationNode::Pointer> entries_;
};

} // namespace native_configuration
} // namespace action_graph

#endif // ACTION_GRAPH_SRC_NATIVE_CONFIGURATION_INCLUDE_NATIVE_CONFIGURATION_MAP_NODE_H_
