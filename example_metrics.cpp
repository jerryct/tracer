// SPDX-License-Identifier: MIT

#include "jerryct/telemetry/counter.h"
#include "jerryct/telemetry/open_metrics_exporter.h"
#include <thread>

int main() {
  jerryct::telemetry::Counter c{jerryct::telemetry::Meter(), "foo"};

  std::thread t{[c]() mutable {
    for (int i = 0; i < 100; ++i) {
      c.Add();
    }
  }};

  for (int i = 0; i < 10; ++i) {
    c.Add();
  }

  t.join();

  jerryct::telemetry::OpenMetricsExporter p{};
  jerryct::telemetry::Meter().Export(p);
  p.Expose();

  return 0;
}
