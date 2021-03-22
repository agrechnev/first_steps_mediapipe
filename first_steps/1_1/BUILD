# You can put comments in BUILD
# target 1_1 : Here we compile main.cpp into a cc_binary
# All MP dependencies should be added explicitly, as shown below

cc_binary(
    name="1_1",
    srcs=["main.cpp"],
    deps = [
        "//mediapipe/calculators/core:pass_through_calculator",
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework/port:parse_text_proto",
    ],
)