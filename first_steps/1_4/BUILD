# This is a macro to compile proto files basically
load("//mediapipe/framework/port:build_config.bzl", "mediapipe_proto_library")
mediapipe_proto_library(
    name = "goblin_calculator14_proto",
    srcs = ["goblin_calculator14.proto"],
    deps = [
        "//mediapipe/framework:calculator_options_proto",
        "//mediapipe/framework:calculator_proto",
    ],
)

# This is our main target
# NOte the dependency name: goblin_calculator14_cc_proto (NOT goblin_calculator14_proto !)
cc_binary(
    name="1_4",
    srcs=["main.cpp", "goblin_calculator14.cpp"],
    deps = [
        ":goblin_calculator14_cc_proto",
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework/port:parse_text_proto",
    ],
)
