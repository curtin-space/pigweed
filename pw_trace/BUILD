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

load(
    "//pw_build:pigweed.bzl",
    "pw_cc_library",
    "pw_cc_test",
)

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # Apache License 2.0

# TODO(pwbug/101): Need to add support for facades/backends to Bazel.

pw_cc_library(
    name = "facade",
    hdrs = [
        "public/pw_trace/trace.h",
        "public/pw_trace/internal/trace_internal.h",
    ],
    includes = ["public"],
    deps = [
        "//pw_preprocessor",
    ],
)

pw_cc_library(
    name = "pw_trace",
    deps = [
        ":facade",
    ],
)

pw_cc_library(
    name = "backend",
    deps = [],
)

pw_cc_test(
    name = "trace_backend_compile_test",
    srcs = [
        "trace_backend_compile_test.cc",
        "trace_backend_compile_test_c.c",
    ],
    deps = [
        ":backend",
        ":facade",
        ":pw_trace",
        "//pw_preprocessor",
        "//pw_unit_test",
    ],
)

pw_cc_test(
    name = "trace_facade_test",
    srcs = [
        "trace_facade_test.cc",
        "pw_trace_test/fake_backend.h",
        "pw_trace_test/public_overrides/pw_trace_backend/trace_backend.h",
    ],
    includes = [
        "pw_trace_test",
        "pw_trace_test/public_overrides"
    ],
    deps = [
        ":backend",
        ":facade",
        ":pw_trace",
        "//pw_preprocessor",
        "//pw_unit_test",
    ],
)
