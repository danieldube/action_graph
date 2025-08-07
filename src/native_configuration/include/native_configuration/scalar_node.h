// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#ifndef ACTION_GRAPH_SRC_NATIVE_CONFIGURATION_INCLUDE_NATIVE_CONFIGURATION_SCALAR_NODE_H_
#define ACTION_GRAPH_SRC_NATIVE_CONFIGURATION_INCLUDE_NATIVE_CONFIGURATION_SCALAR_NODE_H_

#include <action_graph/builder/configuration_node.h>

namespace action_graph {
namespace native_configuration {
class ScalarNode : public builder::ConfigurationNode {
public:
  explicit ScalarNode(std::string value) : value_(std::move(value)) {}
  bool IsScalar() const noexcept override { return true; }
  bool IsMap() const noexcept override { return false; }
  bool IsSequence() const noexcept override { return false; }

  bool HasKey(const std::string &key) const noexcept override { return false; }
  Reference Get(const std::string &key) const override {
    throw ConfigurationNodeNotFound("ScalarNode does not support key access",
                                    *this);
  }
  Reference Get(std::size_t index) const override {
    throw ConfigurationNodeNotFound("ScalarNode does not support index access",
                                    *this);
  }
  std::size_t Size() const noexcept override { return 0; }
  std::string AsString() const noexcept override { return value_; }

private:
  std::string value_;
};

} // namespace native_configuration
} // namespace action_graph

#endif // ACTION_GRAPH_SRC_NATIVE_CONFIGURATION_INCLUDE_NATIVE_CONFIGURATION_SCALAR_NODE_H_
