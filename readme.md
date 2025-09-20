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
