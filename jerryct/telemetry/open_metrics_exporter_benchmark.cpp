// SPDX-License-Identifier: MIT

#include "jerryct/telemetry/counter.h"
#include "jerryct/telemetry/open_metrics_exporter.h"
#include <benchmark/benchmark.h>

namespace {

void ExportOpenMetrics(benchmark::State &state) {
  jerryct::telemetry::OpenMetricsExporter exporter{};

  jerryct::telemetry::Counter c{jerryct::telemetry::Meter(), std::string(64, 'c')};
  for (auto _ : state) {
    c.Add();
    jerryct::telemetry::Meter().Export(exporter);
    benchmark::ClobberMemory();
  }
}

BENCHMARK(ExportOpenMetrics);

} // namespace
