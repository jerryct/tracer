cc_library(
    name = "tracing",
    srcs = [
        "jerryct/tracing/chrome_trace_event_exporter.cpp",
        "jerryct/tracing/prometheus_exporter.cpp",
        "jerryct/tracing/r_exporter.cpp",
        "jerryct/tracing/stats_exporter.cpp",
        "jerryct/tracing/tracing.cpp",
    ],
    hdrs = [
        "jerryct/tracing/chrome_trace_event_exporter.h",
        "jerryct/tracing/prometheus_exporter.h",
        "jerryct/tracing/r_exporter.h",
        "jerryct/tracing/stats_exporter.h",
        "jerryct/tracing/tracing.h",
    ],
    copts = ["-pthread"],
    linkopts = ["-pthread"],
    deps = [
        "@jerryct_string_view//:string_view",
        "@fmtlib_fmt//:fmt",
    ],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "test",
    srcs = [
        "jerryct/tracing/tracing_tests.cpp",
    ],
    deps = [
        ":tracing",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_binary(
    name = "benchmark",
    srcs = [
        "jerryct/tracing/chrome_trace_event_exporter_benchmark.cpp",
        "jerryct/tracing/prometheus_exporter_benchmark.cpp",
        "jerryct/tracing/stats_exporter_benchmark.cpp",
        "jerryct/tracing/tracing_benchmark.cpp",
    ],
    deps = [
        ":tracing",
        "@com_google_benchmark//:benchmark_main",
    ],
)

cc_binary(
    name = "example",
    srcs = [
        "example.cpp",
    ],
    deps = [
        ":tracing",
    ],
)
