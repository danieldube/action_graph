#include "callback_action.h"

CallbackAction::CallbackAction(
    std::string name, std::string message,
    std::function<void(const std::string &)> callback)
    : action_graph::Action(std::move(name)), message_(std::move(message)),
      callback_(std::move(callback)) {}

void CallbackAction::Execute() { callback_(message_); }

std::unique_ptr<action_graph::Action> CreateCallbackActionFromYaml(
    const YAML::Node &node,
    std::function<void(const std::string &message)> callback) {
  return std::make_unique<CallbackAction>(node["name"].as<std::string>(),
                                          node["message"].as<std::string>(),
                                          std::move(callback));
}
