cc_library(
    name = "telemetry",
    srcs = [
        "jerryct/telemetry/chrome_trace_event_exporter.cpp",
        "jerryct/telemetry/counter.cpp",
        "jerryct/telemetry/delta_counter_exporter.cpp",
        "jerryct/telemetry/http_server.cpp",
        "jerryct/telemetry/open_metrics_exporter.cpp",
        "jerryct/telemetry/r_exporter.cpp",
        "jerryct/telemetry/span.cpp",
        "jerryct/telemetry/stats_exporter.cpp",
    ],
    hdrs = [
        "jerryct/telemetry/chrome_trace_event_exporter.h",
        "jerryct/telemetry/counter.h",
        "jerryct/telemetry/delta_counter_exporter.h",
        "jerryct/telemetry/fixed_string.h",
        "jerryct/telemetry/http_server.h",
        "jerryct/telemetry/lock_free_queue.h",
        "jerryct/telemetry/meter.h",
        "jerryct/telemetry/open_metrics_exporter.h",
        "jerryct/telemetry/r_exporter.h",
        "jerryct/telemetry/span.h",
        "jerryct/telemetry/stats_exporter.h",
        "jerryct/telemetry/thread_storage.h",
        "jerryct/telemetry/tracer.h",
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
        "jerryct/telemetry/chrome_trace_event_exporter_tests.cpp",
        "jerryct/telemetry/counter_tests.cpp",
        "jerryct/telemetry/delta_counter_exporter_tests.cpp",
        "jerryct/telemetry/lock_free_queue_tests.cpp",
        "jerryct/telemetry/span_tests.cpp",
    ],
    deps = [
        ":telemetry",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_binary(
    name = "benchmark",
    srcs = [
        "jerryct/telemetry/chrome_trace_event_exporter_benchmark.cpp",
        "jerryct/telemetry/counter_benchmark.cpp",
        "jerryct/telemetry/lock_free_queue_benchmark.cpp",
        "jerryct/telemetry/open_metrics_exporter_benchmark.cpp",
        "jerryct/telemetry/span_benchmark.cpp",
        "jerryct/telemetry/stats_exporter_benchmark.cpp",
        "jerryct/telemetry/tracer_benchmark.cpp",
    ],
    deps = [
        ":telemetry",
        "@com_google_benchmark//:benchmark_main",
    ],
)

cc_binary(
    name = "example_tracing",
    srcs = [
        "example_tracing.cpp",
    ],
    deps = [
        ":telemetry",
    ],
)

cc_binary(
    name = "example_metrics",
    srcs = [
        "example_metrics.cpp",
    ],
    deps = [
        ":telemetry",
    ],
)
