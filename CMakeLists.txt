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

cmake_minimum_required(VERSION 3.14)

include(pw_build/pigweed.cmake)

add_subdirectory(pw_cpu_exception)
add_subdirectory(pw_cpu_exception_armv7m)
add_subdirectory(pw_dumb_io)
add_subdirectory(pw_dumb_io_stdio)
add_subdirectory(pw_preprocessor)
add_subdirectory(pw_span)
add_subdirectory(pw_status)
add_subdirectory(pw_string)
add_subdirectory(pw_unit_test)
add_subdirectory(pw_varint)