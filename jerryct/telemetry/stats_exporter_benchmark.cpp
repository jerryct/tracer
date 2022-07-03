// SPDX-License-Identifier: MIT

#include "jerryct/telemetry/span.h"
#include "jerryct/telemetry/stats_exporter.h"
#include <benchmark/benchmark.h>

namespace {

void ExportStats(benchmark::State &state) {
  jerryct::telemetry::StatsExporter stats{};

  auto name = std::string(64, 'c');
  for (auto _ : state) {
    jerryct::telemetry::Span s{jerryct::telemetry::Tracer(), name};
    jerryct::telemetry::Tracer().Export(stats);
  }
}

BENCHMARK(ExportStats);

} // namespace
