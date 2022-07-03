// SPDX-License-Identifier: MIT

#include "jerryct/telemetry/counter.h"
#include <benchmark/benchmark.h>

namespace {

void Counter(benchmark::State &state) {
  jerryct::telemetry::Counter c{jerryct::telemetry::Meter(), std::string(64, 'c')};
  for (auto _ : state) {
    c.Add();
    jerryct::telemetry::Meter().PerThreadEvents()->ConsumeAll([](auto /*unused*/) {});
    benchmark::DoNotOptimize(jerryct::telemetry::Meter().PerThreadEvents());
    benchmark::ClobberMemory();
  }
}

BENCHMARK(Counter);

} // namespace
