cc_library(
    name = "tracing",
    srcs = [
        "jerryct/tracing/chrome_trace_event_exporter.cpp",
        "jerryct/tracing/counter.cpp",
        "jerryct/tracing/open_metrics_exporter.cpp",
        "jerryct/tracing/r_exporter.cpp",
        "jerryct/tracing/span.cpp",
        "jerryct/tracing/stats_exporter.cpp",
    ],
    hdrs = [
        "jerryct/tracing/chrome_trace_event_exporter.h",
        "jerryct/tracing/counter.h",
        "jerryct/tracing/fixed_string.h",
        "jerryct/tracing/lock_free_queue.h",
        "jerryct/tracing/meter.h",
        "jerryct/tracing/open_metrics_exporter.h",
        "jerryct/tracing/r_exporter.h",
        "jerryct/tracing/span.h",
        "jerryct/tracing/stats_exporter.h",
        "jerryct/tracing/thread_storage.h",
        "jerryct/tracing/tracer.h",
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
        "jerryct/tracing/chrome_trace_event_exporter_tests.cpp",
        "jerryct/tracing/counter_tests.cpp",
        "jerryct/tracing/lock_free_queue_tests.cpp",
        "jerryct/tracing/span_tests.cpp",
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
        "jerryct/tracing/counter_benchmark.cpp",
        "jerryct/tracing/lock_free_queue_benchmark.cpp",
        "jerryct/tracing/open_metrics_exporter_benchmark.cpp",
        "jerryct/tracing/span_benchmark.cpp",
        "jerryct/tracing/stats_exporter_benchmark.cpp",
        "jerryct/tracing/tracer_benchmark.cpp",
    ],
    deps = [
        ":tracing",
        "@com_google_benchmark//:benchmark_main",
    ],
)

cc_binary(
    name = "example_tracing",
    srcs = [
        "example_tracing.cpp",
    ],
    deps = [
        ":tracing",
    ],
)

cc_binary(
    name = "example_metrics",
    srcs = [
        "example_metrics.cpp",
    ],
    deps = [
        ":tracing",
    ],
)
