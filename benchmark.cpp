// SPDX-License-Identifier: MIT

#include "jerryct/tracing/tracing.h"
#include <benchmark/benchmark.h>

namespace {

void PerThreadEvents(benchmark::State &state) {
  for (auto _ : state) {
    auto m = jerryct::trace::Tracer().PerThreadEvents();
    benchmark::DoNotOptimize(m);
    benchmark::ClobberMemory();
  }
}

void Span(benchmark::State &state) {
  auto name = std::string(64, 'c');
  for (auto _ : state) {
    jerryct::trace::Span s{jerryct::trace::Tracer(), name};
    jerryct::trace::Tracer().PerThreadEvents()->events.consumer_all([](auto) {});
    benchmark::DoNotOptimize(jerryct::trace::Tracer().PerThreadEvents());
    benchmark::ClobberMemory();
  }
}

void LockFreeQueue(benchmark::State &state) {
  jerryct::trace::LockFreeQueue<int, 4> r{};
  for (auto _ : state) {
    r.emplace(1);
    r.emplace(2);
    r.emplace(3);
    r.consumer_all([](int) {});
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
BENCHMARK(LockFreeQueue);
BENCHMARK(Now);
BENCHMARK(ClockGettime);

} // namespace

BENCHMARK_MAIN();
