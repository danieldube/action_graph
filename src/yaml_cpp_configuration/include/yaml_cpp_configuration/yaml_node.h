// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#ifndef ACTION_GRAPH_SRC_YAML_CPP_CONFIGURATION_INCLUDE_YAML_CPP_CONFIGURATION_YAML_NODE_H_
#define ACTION_GRAPH_SRC_YAML_CPP_CONFIGURATION_INCLUDE_YAML_CPP_CONFIGURATION_YAML_NODE_H_

#include <action_graph/builder/configuration_node.h>
#include <map>
#include <stdexcept>
#include <string>
#include <yaml-cpp/yaml.h>

namespace action_graph {
namespace yaml_cpp_configuration {

class Node final : public action_graph::builder::ConfigurationNode {
public:
  bool IsScalar() const noexcept override;
  bool IsMap() const noexcept override;
  bool IsSequence() const noexcept override;
  bool HasKey(const std::string &key) const noexcept override;
  Reference Get(const std::string &key) const override;
  Reference Get(size_t index) const override;
  std::size_t Size() const noexcept override;
  std::string AsString() const noexcept override;

  static Node CreateFromString(const std::string &yaml_string);

private:
  explicit Node(const YAML::Node &node);
  YAML::Node node_;
  mutable std::map<std::string, Node> map_{};
  mutable std::map<size_t, Node> sequence_{};
};
} // namespace yaml_cpp_configuration
} // namespace action_graph

#endif // ACTION_GRAPH_SRC_YAML_CPP_CONFIGURATION_INCLUDE_YAML_CPP_CONFIGURATION_YAML_NODE_H_
