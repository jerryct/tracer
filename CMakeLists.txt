cmake_minimum_required(VERSION 3.10)

project(tracer CXX)
enable_testing()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS Off)
option(JERRYCT_TRACER_ENABLE_TESTING "Enable unit tests." ON)
option(JERRYCT_TRACER_ENABLE_BENCHMARK "Enable benchmarks." ON)

set(DEFAULT_COPTS "-fno-omit-frame-pointer" "-g") # "-fno-builtin-memcpy" "-U_FORTIFY_SOURCE"
set(DEFAULT_LINKOPTS "")

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64" OR "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "AMD64")
  list(APPEND DEFAULT_COPTS
    $<$<CONFIG:Debug>:-fsanitize=address,undefined,leak>
    #$<$<CONFIG:Debug>:-fsanitize=thread>
    #$<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:Debug>>:-fprofile-instr-generate>
    #$<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:Debug>>:-fcoverage-mapping>
    $<$<CXX_COMPILER_ID:GNU>:-ftree-vectorize>
    $<$<CXX_COMPILER_ID:GNU>:-fopt-info-vec-optimized>
    -march=nehalem
  )
  list(APPEND DEFAULT_LINKOPTS
    $<$<CONFIG:Debug>:-fsanitize=address,undefined,leak>
    #$<$<CONFIG:Debug>:-fsanitize=thread>
    #$<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:Debug>>:-fprofile-instr-generate>
    #$<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:Debug>>:-fcoverage-mapping>
  )
elseif("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "arm.*|aarch64")
else()
  message(WARNING "Value of CMAKE_SYSTEM_PROCESSOR (${CMAKE_SYSTEM_PROCESSOR}) is unknown")
endif()

add_compile_options(
  ${DEFAULT_COPTS}
)
link_libraries(
  ${DEFAULT_LINKOPTS}
)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "")
add_subdirectory(../benchmark _build/benchmark)
add_subdirectory(../fmt _build/fmt)
add_subdirectory(../googletest _build/googletest)
set(JERRYCT_STRING_VIEW_ENABLE_TESTING OFF CACHE BOOL "")
add_subdirectory(../string_view _build/string_view)

add_library(telemetry
  jerryct/telemetry/chrome_trace_event_exporter.cpp
  jerryct/telemetry/chrome_trace_event_exporter.h
  jerryct/telemetry/counter.cpp
  jerryct/telemetry/counter.h
  jerryct/telemetry/fixed_string.h
  jerryct/telemetry/lock_free_queue.h
  jerryct/telemetry/meter.h
  jerryct/telemetry/open_metrics_exporter.cpp
  jerryct/telemetry/open_metrics_exporter.h
  jerryct/telemetry/r_exporter.cpp
  jerryct/telemetry/r_exporter.h
  jerryct/telemetry/span.cpp
  jerryct/telemetry/span.h
  jerryct/telemetry/stats_exporter.cpp
  jerryct/telemetry/stats_exporter.h
  jerryct/telemetry/thread_storage.h
  jerryct/telemetry/tracer.h
)
target_include_directories(telemetry PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:.>
)
target_link_libraries(telemetry PUBLIC jerryct::string_view Threads::Threads fmt::fmt)
target_compile_options(telemetry PRIVATE "-Wall" "-Wextra" "-Wpedantic" "-Wformat=2" "-Wconversion")

add_executable(example_tracing
  example_tracing.cpp
)
target_link_libraries(example_tracing PRIVATE telemetry)
target_compile_options(example_tracing PRIVATE "-Wall" "-Wextra" "-Wpedantic" "-Wformat=2" "-Wconversion")

add_executable(example_metrics
  example_metrics.cpp
)
target_link_libraries(example_metrics PRIVATE telemetry)
target_compile_options(example_metrics PRIVATE "-Wall" "-Wextra" "-Wpedantic" "-Wformat=2" "-Wconversion")

if (JERRYCT_TRACER_ENABLE_TESTING)
  add_executable(unit_tests
    jerryct/telemetry/chrome_trace_event_exporter_tests.cpp
    jerryct/telemetry/counter_tests.cpp
    jerryct/telemetry/lock_free_queue_tests.cpp
    jerryct/telemetry/span_tests.cpp
  )
  target_link_libraries(unit_tests PRIVATE telemetry gtest_main)
  target_compile_options(unit_tests PRIVATE "-Wall" "-Wextra" "-Wpedantic" "-Wformat=2" "-Wconversion")
  add_test(unit_tests unit_tests)
endif()

if (JERRYCT_TRACER_ENABLE_BENCHMARK)
  add_executable(benchmarks
      jerryct/telemetry/chrome_trace_event_exporter_benchmark.cpp
      jerryct/telemetry/clock_benchmark.cpp
      jerryct/telemetry/counter_benchmark.cpp
      jerryct/telemetry/lock_free_queue_benchmark.cpp
      jerryct/telemetry/open_metrics_exporter_benchmark.cpp
      jerryct/telemetry/span_benchmark.cpp
      jerryct/telemetry/stats_exporter_benchmark.cpp
      jerryct/telemetry/tracer_benchmark.cpp
  )
  target_link_libraries(benchmarks PRIVATE telemetry benchmark::benchmark_main)
  target_compile_options(benchmarks PRIVATE "-Wall" "-Wextra" "-Wpedantic" "-Wformat=2" "-Wconversion")
endif()
