// SPDX-License-Identifier: MIT

#include "jerryct/tracing/prometheus_exporter.h"
#include "jerryct/tracing/tracing.h"
#include <benchmark/benchmark.h>

namespace {

void ExportPrometheus(benchmark::State &state) {
  jerryct::trace::PrometheusExporter prom{};
  fmt::memory_buffer content;
  content.reserve(1024);

  jerryct::trace::Counter c{jerryct::trace::Meter(), std::string(64, 'c')};
  for (auto _ : state) {
    c.Add();
    jerryct::trace::Meter().Export(prom);
    content.clear();
    prom.Expose(content);
    benchmark::DoNotOptimize(content);
    benchmark::ClobberMemory();
  }
}

BENCHMARK(ExportPrometheus);

} // namespace
