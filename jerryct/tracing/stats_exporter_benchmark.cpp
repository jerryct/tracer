// SPDX-License-Identifier: MIT

#include "jerryct/tracing/stats_exporter.h"
#include "jerryct/tracing/tracing.h"
#include <benchmark/benchmark.h>

namespace {

void ExportStats(benchmark::State &state) {
  jerryct::trace::StatsExporter stats{};

  auto name = std::string(64, 'c');
  for (auto _ : state) {
    jerryct::trace::Span s{jerryct::trace::Tracer(), name};
    jerryct::trace::Tracer().Export(stats);
  }
}

BENCHMARK(ExportStats);

} // namespace
