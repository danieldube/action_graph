# Introduction
Action Graph is a small C++ toolkit for wiring together repeatable jobs in a
clear and predictable way. At its heart is the simple `Action` interface, which
lets you describe a unit of work and give it a friendly name so that bigger
flows stay readable. See [`action.h`](src/action_graph/include/action_graph/action.h#L13-L21).

The library ships with helpful building blocks:

* **Reusable action types** – compose work either in order with
  `ActionSequence`, in parallel with `ParallelActions`, or wrap a single
  callable with `SingleAction`, so you can express anything from quick chores
  to fan-out pipelines in a couple of lines. See [`action_sequence.h`](src/action_graph/include/action_graph/action_sequence.h#L18-L35), [`parallel_actions.h`](src/action_graph/include/action_graph/parallel_actions.h#L15-L38), [`single_action.h`](src/action_graph/include/action_graph/single_action.h#L13-L21).
* **Safety and insight via decorators** – plug in observers that watch an
  action start, finish, or fail, or add timing guards that warn when something
  runs too long or misses its expected trigger. See [`decorated_action.h`](src/action_graph/include/action_graph/decorators/decorated_action.h#L15-L31), [`observable_action.h`](src/action_graph/include/action_graph/decorators/observable_action.h#L15-L35), [`timing_monitor.h`](src/action_graph/include/action_graph/decorators/timing_monitor.h#L17-L56), [`execution_observer.h`](src/action_graph/include/action_graph/decorators/execution_observer.h#L13-L26).
* **A global timer** – schedule actions on shared background threads with a
  single timer that triggers callbacks at fixed periods, copes with clock jumps,
  and exposes a simple wait method while it finishes outstanding callbacks. See [`global_timer.h`](src/action_graph/include/action_graph/global_timer/global_timer.h#L41-L127), [`trigger.h`](src/action_graph/include/action_graph/global_timer/trigger.h#L18-L34).
* **Configuration-driven workflows** – feed YAML (or any other implementation
  of the `ConfigurationNode` interface) into the generic builders to parse
  durations, create action trees, apply decorators, and register them with the
  global timer. See [`configuration_node.h`](src/action_graph/include/action_graph/builder/configuration_node.h#L17-L44), [`builder.h`](src/action_graph/include/action_graph/builder/builder.h#L44-L79), [`parse_duration.cpp`](src/action_graph/builder/parse_duration.cpp#L11-L22), [`generic_action_builder.cpp`](src/action_graph/builder/generic_action_builder.cpp#L16-L74), [`generic_action_decorator.h`](src/action_graph/include/action_graph/builder/generic_action_decorator.h#L23-L70), [`yaml_node.h`](src/yaml_cpp_configuration/include/yaml_cpp_configuration/yaml_node.h#L18-L35), [`map_node.h`](src/native_configuration/include/native_configuration/map_node.h#L15-L68), [`sequence_node.h`](src/native_configuration/include/native_configuration/sequence_node.h#L15-L55), [`scalar_node.h`](src/native_configuration/include/native_configuration/scalar_node.h#L13-L33).
* **Pluggable logging** – send human-friendly status updates and errors to any
  sink you like by implementing the lightweight `Log` interface, or use the
  ready-made `FileLog` helper to timestamp entries in a stream. See [`log.h`](src/action_graph/include/action_graph/log.h#L13-L18), [`file_log.h`](src/file_log/include/file_log/file_log.h#L15-L48).

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

## Examples

The `action_graph_examples` target demonstrates four expressive scenarios built
on top of YAML configurations. Build and run them with:

```bash
cmake -S . -B build
cmake --build build --target action_graph_examples
./build/src/examples/action_graph_examples
```

Each run prints the YAML that drives the example, narrates every executed
action, and finishes with a short summary.

### One action every second

This scenario schedules a single action that fires once per second and records
how many reminders reached the console.

```yaml
- trigger:
    name: heartbeat
    period: 1 seconds
    action:
      name: narrate_heartbeat
      type: log_message
      message: "A gentle reminder that another second passed."
```

### Three actions every ten milliseconds

Three parallel actions respond to a high-frequency trigger. The summary counts
how often each sensor-like action produced a message.

```yaml
- trigger:
    name: telemetry_burst
    period: 10 milliseconds
    action:
      name: fan_out_samples
      type: parallel_actions
      actions:
        - action:
            name: log_left_sensor
            type: log_message
            message: "Left sensor captured a sample."
        - action:
            name: log_right_sensor
            type: log_message
            message: "Right sensor captured a sample."
        - action:
            name: log_central_unit
            type: log_message
            message: "Central unit correlated the readings."
```

### Sequential and parallel graph executed once

This example builds a mixed action graph, executes it one time, and shows how
sequential and parallel steps combine to accomplish a larger task.

```yaml
action:
  name: orchestrate_data_flow
  type: sequential_actions
  actions:
    - action:
        name: announce_start
        type: log_message
        message: "Announcing a composed action graph."
    - action:
        name: fetch_inputs
        type: parallel_actions
        actions:
          - action:
              name: pull_profiles
              type: wait_and_log
              message: "Retrieving customer profiles"
              duration: 30 milliseconds
          - action:
              name: pull_orders
              type: wait_and_log
              message: "Collecting recent orders"
              duration: 40 milliseconds
    - action:
        name: aggregate_results
        type: log_message
        message: "Aggregated results ready for presentation."
```

### Timer-driven graph with a timing monitor

The final example decorates a sequential graph with the timing monitor
decorator. When the monitored action exceeds the configured duration limit, the
monitor logs the issue alongside the normal action output.

```yaml
- trigger:
    name: monitored_pipeline
    period: 50 milliseconds
    action:
      name: monitored_sequence
      type: sequential_actions
      decorate:
        - type: timing_monitor
          duration_limit: 40 milliseconds
          expected_period: 50 milliseconds
      actions:
        - action:
            name: capture_measurement
            type: log_message
            message: "Captured a measurement from the probe."
        - action:
            name: publish_with_delay
            type: wait_and_log
            message: "Publishing measurement to observers"
            duration: 60 milliseconds
```
