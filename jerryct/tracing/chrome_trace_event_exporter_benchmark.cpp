// SPDX-License-Identifier: MIT

#include "jerryct/tracing/chrome_trace_event_exporter.h"
#include "jerryct/tracing/tracing.h"
#include <benchmark/benchmark.h>

namespace {

void ExportChromeTrace(benchmark::State &state) {
  jerryct::telemetry::ChromeTraceEventExporter chrome{"test.json"};

  auto name = std::string(64, 'c');
  for (auto _ : state) {
    jerryct::telemetry::Span s{jerryct::telemetry::Tracer(), name};
    jerryct::telemetry::Tracer().Export(chrome);
  }
}

BENCHMARK(ExportChromeTrace);

} // namespace
