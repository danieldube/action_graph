// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#include <action_graph/builder/parse_duration.h>
#include <stdexcept>

namespace action_graph {
namespace builder {
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
