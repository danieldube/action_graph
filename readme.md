# Build

```bash
sudo apt install python3-venv
python3 -m venv ~/python/action_graph
source ~/python/action_graph/bin/activate
pip install -r requirements.txt
pre-commit install
```

## Library usage examples

The `action_graph_examples` target demonstrates how to compose actions and
triggers with expressive YAML configurations. Build and run the executable with:

```bash
cmake -S . -B build
cmake --build build --target action_graph_examples
./build/src/examples/action_graph_examples
```

Each example prints its progress to the console, then emits a short summary
highlighting how often every action ran and when it executed. The YAML strings
shown below mirror the in-code configurations so they can be reused outside of
C++ if desired.

### One action triggered every second

This configuration schedules a single heartbeat action once per second. The
`log_message` action prints a friendly status line whenever the trigger fires,
and the summary reports the measured timing between events.

```yaml
- trigger:
    name: heartbeat
    period: 1 seconds
    action:
      name: heartbeat_action
      type: log_message
      message: "Heartbeat action executed."
```

### Three actions triggered every 10 milliseconds

Three independent triggers share the same 10 ms period. Each trigger invokes a
dedicated action so that the output highlights concurrent high-frequency
processing and the summary shows how many cycles each action completed.

```yaml
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
```

### Parallel and sequential graph triggered once

This example executes a mixed graph exactly once: a sequential workflow wraps a
parallel branch that mixes logging with simulated work (`wait`). The logs show
when each sub-action runs and the summary captures their execution order.

```yaml
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
```

### Timing monitored graph triggered by a timer

The final example wraps a sequential action graph in a `TimingMonitor`. The
monitor reports when execution exceeds the budget or when a trigger misses its
period, so the output demonstrates the monitoring callbacks alongside the
action summary.

```yaml
- trigger:
    name: monitored_job
    period: 50 milliseconds
    action:
      name: monitored_sequence
      type: monitored_action
      monitor:
        duration_limit: 30 milliseconds
        period: 50 milliseconds
        on_duration_exceeded: "Execution exceeded the 30 ms budget."
        on_trigger_miss: "Trigger period was missed."
      action:
        action:
          name: monitored_steps
          type: sequential_actions
          actions:
            - action:
                name: measured_step
                type: wait
                duration: 60 milliseconds
            - action:
                name: announce_completion
                type: log_message
                message: "Monitored sequence finished."
```
