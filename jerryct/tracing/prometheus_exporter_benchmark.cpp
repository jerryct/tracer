// SPDX-License-Identifier: MIT

#include "jerryct/tracing/prometheus_exporter.h"
#include "jerryct/tracing/tracing.h"
#include <benchmark/benchmark.h>

namespace {

void ExportPrometheus(benchmark::State &state) {
  jerryct::trace::PrometheusExporter prom{};
  std::string content;
  content.reserve(1024);

  auto name = std::string(64, 'c');
  for (auto _ : state) {
    jerryct::trace::Span s{jerryct::trace::Tracer(), name};
    jerryct::trace::Tracer().Export(prom);
    content.clear();
    prom.Expose(content);
    benchmark::DoNotOptimize(content);
    benchmark::ClobberMemory();
  }
}

BENCHMARK(ExportPrometheus);

} // namespace
