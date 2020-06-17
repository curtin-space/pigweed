load("@subpar//:subpar.bzl", "par_binary")

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

par_binary(
    name = "plugin",
    main = "plugin.py",
    srcs = [
        "plugin.py",
        "proto_tree.py",
        "codegen_pwpb.py"
    ],
    imports = ["pw_protobuf/py/pw_protobuf"],
    visibility = ["//visibility:public"],
)
