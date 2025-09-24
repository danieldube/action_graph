// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#pragma once

#include <string>

namespace examples {
namespace configurations {

inline std::string OneSecondTriggerYaml() {
  return R"yaml(
- trigger:
    name: heartbeat
    period: 1 seconds
    action:
      name: heartbeat_action
      type: log_message
      message: "Heartbeat action executed."
)yaml";
}

inline std::string ThreeActionsTenMillisecondsYaml() {
  return R"yaml(
- trigger:
    name: alpha
    period: 10 milliseconds
    action:
      name: alpha_action
      type: log_message
      message: "Alpha fired."
- trigger:
    name: beta
    period: 10 milliseconds
    action:
      name: beta_action
      type: log_message
      message: "Beta fired."
- trigger:
    name: gamma
    period: 10 milliseconds
    action:
      name: gamma_action
      type: log_message
      message: "Gamma fired."
)yaml";
}

inline std::string GraphExecutionYaml() {
  return R"yaml(
action:
  name: onboarding_flow
  type: sequential_actions
  actions:
    - action:
        name: introduce
        type: log_message
        message: "Introduce the system to the user."
    - action:
        name: prepare_environment
        type: parallel_actions
        actions:
          - action:
              name: load_configuration
              type: wait
              duration: 5 milliseconds
          - action:
              name: warm_up_cache
              type: log_message
              message: "Cache is being warmed up."
          - action:
              name: notify_team
              type: log_message
              message: "Team notified about onboarding."
    - action:
        name: finalize
        type: log_message
        message: "Onboarding flow completed."
)yaml";
}

inline std::string MonitoredTimerYaml() {
  return R"yaml(
- trigger:
    name: monitored_job
    period: 50 milliseconds
    action:
      name: monitored_steps
      type: sequential_actions
      decorate:
        - type: timing_monitor
          duration_limit: 30 milliseconds
          expected_period: 50 milliseconds
      actions:
        - action:
            name: measured_step
            type: wait
            duration: 60 milliseconds
        - action:
            name: announce_completion
            type: log_message
            message: "Monitored sequence finished."
)yaml";
}

} // namespace configurations
} // namespace examples

