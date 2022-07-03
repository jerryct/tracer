// SPDX-License-Identifier: MIT

#include "jerryct/tracing/lock_free_queue.h"
#include <benchmark/benchmark.h>

namespace {

void LockFreeQueue(benchmark::State &state) {
  jerryct::telemetry::LockFreeQueue<int, 4> r{};
  for (auto _ : state) {
    r.Emplace(1);
    r.Emplace(2);
    r.Emplace(3);
    r.ConsumeAll([](int /*unused*/) {});
    benchmark::ClobberMemory();
  }
}

void LockFreeQueueFull(benchmark::State &state) {
  jerryct::telemetry::LockFreeQueue<int, 1> r{};
  for (auto _ : state) {
    r.Emplace(1);
    benchmark::ClobberMemory();
  }
}

BENCHMARK(LockFreeQueue);
BENCHMARK(LockFreeQueueFull);

} // namespace
