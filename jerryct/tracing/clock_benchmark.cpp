// SPDX-License-Identifier: MIT

#include <benchmark/benchmark.h>
#include <chrono>

namespace {

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

BENCHMARK(Now);
BENCHMARK(ClockGettime);

} // namespace
