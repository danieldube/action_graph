// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#include "example_runners.h"

#include "example_support.h"

#include <iostream>
#include <memory>
#include <string>

namespace {

constexpr char kOnboardingYaml[] = R"yaml(
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

action_graph::builder::ActionObject
BuildOnboardingAction(examples::ExampleSession &session) {
  return session.BuildAction(session.Configuration());
}

void AnnounceOnboardingStart(examples::ExampleSession &session) {
  session.Context().Log(
      "Executing onboarding flow once to observe action order...");
}

void AnnounceOnboardingFinish(examples::ExampleSession &session) {
  session.Context().Log("Onboarding flow execution finished.");
}

void ExecuteOnboardingFlow(examples::ExampleSession &session,
                           action_graph::builder::ActionObject &action) {
  AnnounceOnboardingStart(session);
  action->Execute();
  AnnounceOnboardingFinish(session);
}

} // namespace

namespace examples {

void RunGraphExecutionExample() {
  ExampleSession session(std::cout,
                         "Parallel and sequential graph executed once",
                         kOnboardingYaml);
  auto action = BuildOnboardingAction(session);
  ExecuteOnboardingFlow(session, action);
}

} // namespace examples
