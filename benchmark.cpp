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
  auto name = std::string(64, 'c');
  for (auto _ : state) {
    trace::Span s{trace::Tracer(), name};
    trace::Tracer().PerThreadEvents()->events.consumer_all([](auto) {});
    benchmark::DoNotOptimize(trace::Tracer().PerThreadEvents());
    benchmark::ClobberMemory();
  }
}

void LockFreeQueue(benchmark::State &state) {
  trace::LockFreeQueue<int, 4> r{};
  for (auto _ : state) {
    r.emplace(1);
    r.emplace(2);
    r.emplace(3);
    r.consumer_all([](int) {});
    benchmark::ClobberMemory();
  }
}

BENCHMARK(PerThreadEvents);
BENCHMARK(Span);
BENCHMARK(LockFreeQueue);

} // namespace

BENCHMARK_MAIN();
