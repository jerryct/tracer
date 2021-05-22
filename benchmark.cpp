// SPDX-License-Identifier: MIT

#include "tracing/tracing.h"
#include <benchmark/benchmark.h>

namespace {

void PerThreadEvents(benchmark::State &state) {
  for (auto _ : state) {
    auto m = trace::Tracer().PerThreadEvents();
    benchmark::DoNotOptimize(m);
    benchmark::ClobberMemory();
  }
}

void Span(benchmark::State &state) {
  auto m = trace::Tracer().RegisterDurationEvent("main", "tag");
  for (auto _ : state) {
    trace::Span s{trace::Tracer(), m};
    benchmark::DoNotOptimize(trace::Tracer().PerThreadEvents());
    benchmark::ClobberMemory();
  }
}

void Register(benchmark::State &state) {
  for (auto _ : state) {
    trace::Tracer().RegisterDurationEvent("main", "tag");
    benchmark::DoNotOptimize(trace::Tracer().attributes_);
    benchmark::ClobberMemory();
  }
}

BENCHMARK(PerThreadEvents);
BENCHMARK(Register);
BENCHMARK(Span);

} // namespace

BENCHMARK_MAIN();
