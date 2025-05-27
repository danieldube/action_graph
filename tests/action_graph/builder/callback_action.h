#ifndef ACTION_GRAPH_TESTS_ACTION_GRAPH_BUILDER_CALLBACK_ACTION_H_
#define ACTION_GRAPH_TESTS_ACTION_GRAPH_BUILDER_CALLBACK_ACTION_H_

#include <action_graph/action.h>
#include <functional>
#include <memory>
#include <string>
#include <yaml-cpp/yaml.h>

class CallbackAction final : public action_graph::Action {
public:
  CallbackAction(std::string name, std::string message,
                 std::function<void(const std::string &)> callback);

  void Execute() override;

private:
  std::string message_;
  std::function<void(const std::string &)> callback_;
};

std::unique_ptr<action_graph::Action> CreateCallbackActionFromYaml(
    const YAML::Node &node,
    std::function<void(const std::string &message)> callback);

#endif // ACTION_GRAPH_TESTS_ACTION_GRAPH_BUILDER_CALLBACK_ACTION_H_
