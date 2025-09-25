// Copyright (c) 2025 Daniel Dube
//
// This file is part of the action_graph library and is licensed under the MIT
// License. See the LICENSE file in the root directory for full license text.

#ifndef ACTION_GRAPH_SRC_EXAMPLES_EXAMPLE_TIMER_RUNNER_H_
#define ACTION_GRAPH_SRC_EXAMPLES_EXAMPLE_TIMER_RUNNER_H_

#include <string_view>

void RunTimedExample(std::string_view title, std::string_view yaml_text,
                     int cycles);
void RunGraphExample(std::string_view title, std::string_view yaml_text);

#endif // ACTION_GRAPH_SRC_EXAMPLES_EXAMPLE_TIMER_RUNNER_H_
