add_library(native_configuration INTERFACE)

target_sources(
  native_configuration
  INTERFACE FILE_SET
            action_graph_headers
            TYPE
            HEADERS
            BASE_DIRS
            ./include
            FILES
            include/native_configuration/map_node.h
            include/native_configuration/scalar_node.h
            include/native_configuration/sequence_node.h)

target_link_libraries(native_configuration INTERFACE action_graph::action_graph)

add_library(action_graph::native_configuration ALIAS native_configuration)
