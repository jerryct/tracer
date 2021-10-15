// SPDX-License-Identifier: MIT

#include "jerryct/tracing/open_metrics_exporter.h"
#include "jerryct/tracing/tracing.h"
#include <thread>

int main() {
  jerryct::trace::Counter c{jerryct::trace::Meter(), "foo"};

  std::thread t{[c]() mutable {
    for (int i = 0; i < 100; ++i) {
      c.Add();
    }
  }};

  for (int i = 0; i < 10; ++i) {
    c.Add();
  }

  t.join();

  jerryct::trace::OpenMetricsExporter p{};
  jerryct::trace::Meter().Export(p);
  p.Expose();

  return 0;
}
