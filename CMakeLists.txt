# Copyright 2020 The Pigweed Authors
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License. You may obtain a copy of
# the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations under
# the License.

project(Pigweed)

cmake_minimum_required(VERSION 3.16)

# The PW_ROOT environment variable should be set in bootstrap. If it is not set,
# set it to this directory.
if("$ENV{PW_ROOT}" STREQUAL "")
  message("The PW_ROOT environment variable is not set; "
          "using ${CMAKE_CURRENT_LIST_DIR} within CMake")
  set(ENV{PW_ROOT} "${CMAKE_CURRENT_LIST_DIR}")
endif()

add_subdirectory(pw_assert EXCLUDE_FROM_ALL)
add_subdirectory(pw_assert_basic EXCLUDE_FROM_ALL)
add_subdirectory(pw_assert_log EXCLUDE_FROM_ALL)
add_subdirectory(pw_base64 EXCLUDE_FROM_ALL)
add_subdirectory(pw_blob_store EXCLUDE_FROM_ALL)
add_subdirectory(pw_build EXCLUDE_FROM_ALL)
add_subdirectory(pw_bytes EXCLUDE_FROM_ALL)
add_subdirectory(pw_checksum EXCLUDE_FROM_ALL)
add_subdirectory(pw_chrono EXCLUDE_FROM_ALL)
add_subdirectory(pw_chrono_stl EXCLUDE_FROM_ALL)
add_subdirectory(pw_containers EXCLUDE_FROM_ALL)
add_subdirectory(pw_cpu_exception EXCLUDE_FROM_ALL)
add_subdirectory(pw_cpu_exception_cortex_m EXCLUDE_FROM_ALL)
add_subdirectory(pw_file EXCLUDE_FROM_ALL)
add_subdirectory(pw_function EXCLUDE_FROM_ALL)
add_subdirectory(pw_hdlc EXCLUDE_FROM_ALL)
add_subdirectory(pw_kvs EXCLUDE_FROM_ALL)
add_subdirectory(pw_log EXCLUDE_FROM_ALL)
add_subdirectory(pw_log_basic EXCLUDE_FROM_ALL)
add_subdirectory(pw_log_null EXCLUDE_FROM_ALL)
add_subdirectory(pw_log_tokenized EXCLUDE_FROM_ALL)
add_subdirectory(pw_minimal_cpp_stdlib EXCLUDE_FROM_ALL)
add_subdirectory(pw_polyfill EXCLUDE_FROM_ALL)
add_subdirectory(pw_protobuf EXCLUDE_FROM_ALL)
add_subdirectory(pw_protobuf_compiler EXCLUDE_FROM_ALL)
add_subdirectory(pw_preprocessor EXCLUDE_FROM_ALL)
add_subdirectory(pw_random EXCLUDE_FROM_ALL)
add_subdirectory(pw_result EXCLUDE_FROM_ALL)
add_subdirectory(pw_ring_buffer EXCLUDE_FROM_ALL)
add_subdirectory(pw_router EXCLUDE_FROM_ALL)
add_subdirectory(pw_rpc EXCLUDE_FROM_ALL)
add_subdirectory(pw_span EXCLUDE_FROM_ALL)
add_subdirectory(pw_status EXCLUDE_FROM_ALL)
add_subdirectory(pw_stream EXCLUDE_FROM_ALL)
add_subdirectory(pw_string EXCLUDE_FROM_ALL)
add_subdirectory(pw_sync EXCLUDE_FROM_ALL)
add_subdirectory(pw_sync_stl EXCLUDE_FROM_ALL)
add_subdirectory(pw_sys_io EXCLUDE_FROM_ALL)
add_subdirectory(pw_sys_io_stdio EXCLUDE_FROM_ALL)
add_subdirectory(pw_tokenizer EXCLUDE_FROM_ALL)
add_subdirectory(pw_trace EXCLUDE_FROM_ALL)
add_subdirectory(pw_trace_tokenized EXCLUDE_FROM_ALL)
add_subdirectory(pw_transfer EXCLUDE_FROM_ALL)
add_subdirectory(pw_unit_test EXCLUDE_FROM_ALL)
add_subdirectory(pw_varint EXCLUDE_FROM_ALL)

add_subdirectory(targets/host EXCLUDE_FROM_ALL)

add_subdirectory(third_party/nanopb EXCLUDE_FROM_ALL)

add_custom_target(pw_apps)
add_dependencies(pw_apps pw_hdlc.rpc_example)
