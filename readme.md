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
