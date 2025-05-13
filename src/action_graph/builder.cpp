#include <action_graph/action.h>
#include <action_graph/builder.h>

namespace action_graph {
namespace builder {

std::vector<ActionObject>
BuildActionGraph(const std::string &yaml_string,
                 const ActionCreators &action_creators) {
  std::vector<ActionObject> created_actions;
  YAML::Node config = YAML::Load(yaml_string);
  auto action = config["trigger"]["action"];
  auto action_type = action["type"].as<std::string>();
  auto creator = action_creators.find(action_type);
  if (creator == action_creators.end()) {
    throw std::runtime_error("Action type not found: " + action_type);
  }
  created_actions.push_back(creator->second(action));
  return created_actions;
}

std::chrono::duration<double> ParseDuration(const std::string &duration_str) {
  if (duration_str.find("nanoseconds") != std::string::npos) {
    return std::chrono::nanoseconds(std::stoi(duration_str));
  } else if (duration_str.find("microseconds") != std::string::npos) {
    return std::chrono::microseconds(std::stoi(duration_str));
  } else if (duration_str.find("milliseconds") != std::string::npos) {
    return std::chrono::milliseconds(std::stoi(duration_str));
  } else if (duration_str.find("seconds") != std::string::npos) {
    return std::chrono::seconds(std::stoi(duration_str));
  } else {
    throw std::invalid_argument("Invalid duration format");
  }
}

} // namespace builder
} // namespace action_graph
