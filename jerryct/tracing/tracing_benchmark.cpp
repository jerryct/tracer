// SPDX-License-Identifier: MIT

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

void Counter(benchmark::State &state) {
  jerryct::trace::Counter c{jerryct::trace::Meter(), std::string(64, 'c')};
  for (auto _ : state) {
    c.Increment();
    jerryct::trace::Meter().PerThreadEvents()->events.ConsumeAll([](auto /*unused*/) {});
    benchmark::DoNotOptimize(jerryct::trace::Meter().PerThreadEvents());
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

void LockFreeQueueFull(benchmark::State &state) {
  jerryct::trace::LockFreeQueue<int, 1> r{};
  for (auto _ : state) {
    r.Emplace(1);
    benchmark::ClobberMemory();
  }
}

BENCHMARK(PerThreadEvents);
BENCHMARK(Span);
BENCHMARK(Counter);
BENCHMARK(LockFreeQueue);
BENCHMARK(LockFreeQueueFull);

} // namespace
