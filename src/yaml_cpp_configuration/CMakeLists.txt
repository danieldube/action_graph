# Copyright (c) 2025 Daniel Dube
#
# This file is part of the action_graph library and is licensed under the MIT
# License. See the LICENSE file in the root directory for full license text.

find_package(yaml-cpp REQUIRED)

add_library(yaml_cpp_configuration)

target_sources(
  yaml_cpp_configuration
  PRIVATE src/yaml_node.cpp
  PUBLIC FILE_SET
         action_graph_headers
         TYPE
         HEADERS
         BASE_DIRS
         ${CMAKE_CURRENT_SOURCE_DIR}/include
         FILES
         include/yaml_cpp_configuration/yaml_node.h)

target_link_libraries(yaml_cpp_configuration PUBLIC action_graph::action_graph
                                                    yaml-cpp::yaml-cpp)

target_compile_features(yaml_cpp_configuration PUBLIC cxx_std_14)
set_target_properties(yaml_cpp_configuration PROPERTIES CXX_EXTENSIONS OFF)

add_library(action_graph::yaml_cpp_configuration ALIAS yaml_cpp_configuration)
