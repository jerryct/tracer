// SPDX-License-Identifier: MIT

#include "jerryct/telemetry/span.h"
#include <benchmark/benchmark.h>

namespace {

void Span(benchmark::State &state) {
  auto name = std::string(64, 'c');
  for (auto _ : state) {
    jerryct::telemetry::Span s{jerryct::telemetry::Tracer(), name};
    jerryct::telemetry::Tracer().PerThreadEvents()->ConsumeAll([](auto /*unused*/) {});
    benchmark::DoNotOptimize(jerryct::telemetry::Tracer().PerThreadEvents());
    benchmark::ClobberMemory();
  }
}

BENCHMARK(Span);

} // namespace
