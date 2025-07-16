find_package(yaml-cpp REQUIRED)

add_library(yaml_cpp_configuration INTERFACE)

target_sources(
  yaml_cpp_configuration
  INTERFACE FILE_SET
            action_graph_headers
            TYPE
            HEADERS
            BASE_DIRS
            ./include
            FILES
            include/yaml_cpp_configuration/yaml_node.h)

target_link_libraries(yaml_cpp_configuration
                      INTERFACE action_graph::action_graph yaml-cpp::yaml-cpp)

add_library(action_graph::yaml_cpp_configuration ALIAS yaml_cpp_configuration)
