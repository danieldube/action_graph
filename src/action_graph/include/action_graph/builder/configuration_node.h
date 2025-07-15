#ifndef ACTION_GRAPH_SRC_ACTION_GRAPH_BUILDER_CONFIGURATION_NODE_H_
#define ACTION_GRAPH_SRC_ACTION_GRAPH_BUILDER_CONFIGURATION_NODE_H_

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>

namespace action_graph {
namespace builder {

class ConfigurationNode {
public:
  virtual ~ConfigurationNode() = default;

  using Pointer = std::unique_ptr<ConfigurationNode>;
  using Reference = ConfigurationNode &;

  class ConfigurationNodeNotFound : public std::runtime_error {
  public:
    ConfigurationNodeNotFound(const std::string &message,
                              const ConfigurationNode &node)
        : std::runtime_error("Configuration node not found: " + message +
                             "\nNode:" + node.AsString()) {}
    explicit ConfigurationNodeNotFound(const std::string &message)
        : std::runtime_error("Configuration node not found: " + message) {}
  };

  virtual bool IsScalar() const noexcept = 0;
  virtual bool IsMap() const noexcept = 0;
  virtual bool IsSequence() const noexcept = 0;

  virtual bool HasKey(const std::string &key) const noexcept = 0;
  virtual Reference Get(const std::string &key) const = 0;
  virtual Reference Get(std::size_t index) const = 0;
  virtual std::size_t Size() const noexcept = 0;

  virtual std::string AsString() const noexcept = 0;
};
} // namespace builder
} // namespace action_graph

#endif // ACTION_GRAPH_SRC_ACTION_GRAPH_BUILDER_CONFIGURATION_NODE_H_
