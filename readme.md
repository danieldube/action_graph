# Introduction
Action Graph is a small C++ toolkit for wiring together repeatable jobs in a
clear and predictable way. At its heart is the simple `Action` interface, which
lets you describe a unit of work and give it a friendly name so that bigger
flows stay readable.【F:src/action_graph/include/action_graph/action.h†L13-L21】

The library ships with helpful building blocks:

* **Reusable action types** – compose work either in order with
  `ActionSequence`, in parallel with `ParallelActions`, or wrap a single
  callable with `SingleAction`, so you can express anything from quick chores
  to fan-out pipelines in a couple of lines.【F:src/action_graph/include/action_graph/action_sequence.h†L18-L35】【F:src/action_graph/include/action_graph/parallel_actions.h†L15-L38】【F:src/action_graph/include/action_graph/single_action.h†L13-L21】
* **Safety and insight via decorators** – plug in observers that watch an
  action start, finish, or fail, or add timing guards that warn when something
  runs too long or misses its expected trigger.【F:src/action_graph/include/action_graph/decorators/decorated_action.h†L15-L31】【F:src/action_graph/include/action_graph/decorators/observable_action.h†L15-L35】【F:src/action_graph/include/action_graph/decorators/timing_monitor.h†L17-L56】【F:src/action_graph/include/action_graph/decorators/execution_observer.h†L13-L26】
* **A global timer** – schedule actions on shared background threads with a
  single timer that triggers callbacks at fixed periods, copes with clock jumps,
  and exposes a simple wait method while it finishes outstanding callbacks.【F:src/action_graph/include/action_graph/global_timer/global_timer.h†L41-L127】【F:src/action_graph/include/action_graph/global_timer/trigger.h†L18-L34】
* **Configuration-driven workflows** – feed YAML (or any other implementation
  of the `ConfigurationNode` interface) into the generic builders to parse
  durations, create action trees, apply decorators, and register them with the
  global timer.【F:src/action_graph/include/action_graph/builder/configuration_node.h†L17-L44】【F:src/action_graph/include/action_graph/builder/builder.h†L44-L79】【F:src/action_graph/builder/parse_duration.cpp†L11-L22】【F:src/action_graph/builder/generic_action_builder.cpp†L16-L74】【F:src/action_graph/include/action_graph/builder/generic_action_decorator.h†L23-L70】【F:src/yaml_cpp_configuration/include/yaml_cpp_configuration/yaml_node.h†L18-L35】【F:src/native_configuration/include/native_configuration/map_node.h†L15-L68】【F:src/native_configuration/include/native_configuration/sequence_node.h†L15-L55】【F:src/native_configuration/include/native_configuration/scalar_node.h†L13-L33】
* **Pluggable logging** – send human-friendly status updates and errors to any
  sink you like by implementing the lightweight `Log` interface, or use the
  ready-made `FileLog` helper to timestamp entries in a stream.【F:src/action_graph/include/action_graph/log.h†L13-L18】【F:src/file_log/include/file_log/file_log.h†L15-L48】

Together these pieces make it easy to describe what should happen, when it
should happen, and how it should be monitored, all while keeping your own code
focused on the work each action performs.

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
