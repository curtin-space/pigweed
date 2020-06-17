#load("@rules_python//python:defs.bzl", "py_library","py_binary")

py_library(
    name = "proto_tree",
    srcs = ["proto_tree.py"],
)

py_library(
    name = "codegen_pwpb",
    srcs = ["codegen_pwpb.py"],
    deps = [
        ":proto_tree",
    ],
)

py_binary(
    name = "plugin",
    main = "plugin.py",
    srcs = [
        "plugin.py",
        "proto_tree.py",
        "codegen_pwpb.py"
    ],
    visibility = ["//visibility:public"],
)
