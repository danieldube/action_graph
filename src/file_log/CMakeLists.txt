# Copyright (c) 2025 Daniel Dube
#
# This file is part of the action_graph library and is licensed under the MIT
# License. See the LICENSE file in the root directory for full license text.

add_library(file_log INTERFACE)

target_sources(
  file_log
  INTERFACE FILE_SET
            action_graph_headers
            TYPE
            HEADERS
            BASE_DIRS
            ${CMAKE_CURRENT_LIST_DIR}/include
            FILES
            include/file_log/file_log.h)

target_link_libraries(file_log INTERFACE action_graph::action_graph)
add_library(action_graph::file_log ALIAS file_log)
