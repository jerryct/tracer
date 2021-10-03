// SPDX-License-Identifier: MIT

#include "jerryct/tracing/prometheus_exporter.h"
#include "jerryct/tracing/stats_exporter.h"
#include "jerryct/tracing/tracing.h"
#include <benchmark/benchmark.h>

namespace {

void PerThreadEvents(benchmark::State &state) {
  for (auto _ : state) {
    auto *m = jerryct::trace::Tracer().PerThreadEvents();
    benchmark::DoNotOptimize(m);
    benchmark::ClobberMemory();
  }
}

void Span(benchmark::State &state) {
  auto name = std::string(64, 'c');
  for (auto _ : state) {
    jerryct::trace::Span s{jerryct::trace::Tracer(), name};
    jerryct::trace::Tracer().PerThreadEvents()->events.ConsumeAll([](auto /*unused*/) {});
    benchmark::DoNotOptimize(jerryct::trace::Tracer().PerThreadEvents());
    benchmark::ClobberMemory();
  }
}

void ExportStats(benchmark::State &state) {
  jerryct::trace::StatsExporter stats{};

  auto name = std::string(64, 'c');
  for (auto _ : state) {
    jerryct::trace::Span s{jerryct::trace::Tracer(), name};
    jerryct::trace::Tracer().Export(stats);
  }
}

void ExportPrometheus(benchmark::State &state) {
  jerryct::trace::PrometheusExporter prom{};
  std::string content;
  content.reserve(1024);

  auto name = std::string(64, 'c');
  for (auto _ : state) {
    jerryct::trace::Span s{jerryct::trace::Tracer(), name};
    jerryct::trace::Tracer().Export(prom);
    content.clear();
    prom.Expose(content);
    benchmark::DoNotOptimize(content);
    benchmark::ClobberMemory();
  }
}

void LockFreeQueue(benchmark::State &state) {
  jerryct::trace::LockFreeQueue<int, 4> r{};
  for (auto _ : state) {
    r.Emplace(1);
    r.Emplace(2);
    r.Emplace(3);
    r.ConsumeAll([](int /*unused*/) {});
    benchmark::ClobberMemory();
  }
}

void Now(benchmark::State &state) {
  for (auto _ : state) {
    const auto now = std::chrono::steady_clock::now();
    benchmark::DoNotOptimize(now);
    benchmark::ClobberMemory();
  }
}
void ClockGettime(benchmark::State &state) {
  for (auto _ : state) {
    timespec tp;
    // syscall(SYS_clock_gettime, CLOCK_MONOTONIC, &tp);
    clock_gettime(CLOCK_MONOTONIC, &tp);
    const auto now = std::chrono::steady_clock::time_point(
        std::chrono::steady_clock::duration(std::chrono::seconds(tp.tv_sec) + std::chrono::nanoseconds(tp.tv_nsec)));
    benchmark::DoNotOptimize(now);
    benchmark::ClobberMemory();
  }
}

BENCHMARK(PerThreadEvents);
BENCHMARK(Span);
BENCHMARK(ExportStats);
BENCHMARK(ExportPrometheus);
BENCHMARK(LockFreeQueue);
BENCHMARK(Now);
BENCHMARK(ClockGettime);

} // namespace

BENCHMARK_MAIN();
