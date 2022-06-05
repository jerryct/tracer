// SPDX-License-Identifier: MIT

#include "jerryct/tracing/open_metrics_exporter.h"
#include "jerryct/tracing/tracing.h"
#include <benchmark/benchmark.h>

namespace {

void ExportOpenMetrics(benchmark::State &state) {
  jerryct::trace::OpenMetricsExporter exporter{};
  fmt::memory_buffer content;
  content.reserve(1024);

  jerryct::trace::Counter c{jerryct::trace::Meter(), std::string(64, 'c')};
  for (auto _ : state) {
    c.Add();
    jerryct::trace::Meter().Export(exporter);
    content.clear();
    exporter.Expose(content);
    benchmark::DoNotOptimize(content);
    benchmark::ClobberMemory();
  }
}

BENCHMARK(ExportOpenMetrics);

} // namespace
