// SPDX-License-Identifier: MIT

#include "jerryct/telemetry/tracer.h"
#include <benchmark/benchmark.h>

namespace {

void TracerPerThreadEvents(benchmark::State &state) {
  for (auto _ : state) {
    auto *m = jerryct::telemetry::Tracer().PerThreadEvents();
    benchmark::DoNotOptimize(m);
    benchmark::ClobberMemory();
  }
}

BENCHMARK(TracerPerThreadEvents);

} // namespace
