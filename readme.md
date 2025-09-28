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

Build the `action_graph_examples` target to see the library in action:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target action_graph_examples
./build/src/examples/action_graph_examples
```

Each example uses a small YAML configuration to describe its actions. The
`log_action` type used below is defined inside the example binary and simply
writes friendly messages to the console. An optional `delay` field allows the
examples to emulate long-running work.

### One action triggered every second

Registers a single periodic job with the global timer. The job prints a message
once per second.

```yaml
- trigger:
    name: announce_tick
    period: 1 seconds
    action:
      name: log_tick
      type: log_action
      message: "tick"
```

### Three actions triggered every 10 milliseconds

Shows how multiple triggers can share the same period while remaining
independent. Each action records a sampling event.

```yaml
- trigger:
    name: sensor_one
    period: 10 milliseconds
    action:
      name: capture_sensor_one
      type: log_action
      message: "sensor one sampled"
- trigger:
    name: sensor_two
    period: 10 milliseconds
    action:
      name: capture_sensor_two
      type: log_action
      message: "sensor two sampled"
- trigger:
    name: sensor_three
    period: 10 milliseconds
    action:
      name: capture_sensor_three
      type: log_action
      message: "sensor three sampled"
```

### Parallel and sequential graph executed once

Demonstrates a nested action graph: a sequential pipeline that contains a
parallel fan-out step with its own internal sequence. Because the configuration
represents a single graph, it is executed directly without registering it on the
timer.

```yaml
action:
  name: content_pipeline
  type: sequential_actions
  actions:
    - action:
        name: prepare_context
        type: log_action
        message: "prepare context"
    - action:
        name: load_and_process
        type: parallel_actions
        actions:
          - action:
              name: load_assets
              type: log_action
              message: "load assets"
          - action:
              name: process_assets
              type: sequential_actions
              actions:
                - action:
                    name: decode
                    type: log_action
                    message: "decode"
                - action:
                    name: enrich
                    type: log_action
                    message: "enrich"
    - action:
        name: publish
        type: log_action
        message: "publish"
```

### Timer-driven graph decorated with a `TimingMonitor`

Wires a sequence of actions to the timer and decorates it with a
`TimingMonitor` via the `GenericActionDecorator`. The monitor reports when a
cycle exceeds its allotted run time or misses its expected trigger.

```yaml
- trigger:
    name: monitored_cycle
    period: 200 milliseconds
    action:
      name: monitored_sequence
      type: sequential_actions
      decorate:
        - type: timing_monitor
          duration_limit: 120 milliseconds
          expected_period: 200 milliseconds
      actions:
        - action:
            name: start_cycle
            type: log_action
            message: "start cycle"
        - action:
            name: slow_work
            type: log_action
            message: "simulate load"
            delay: 250 milliseconds
        - action:
            name: finish_cycle
            type: log_action
            message: "finish cycle"
```
