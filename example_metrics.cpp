// SPDX-License-Identifier: MIT

#include "jerryct/tracing/prometheus_exporter.h"
#include "jerryct/tracing/tracing.h"
#include <thread>

int main() {
  jerryct::trace::Counter c{jerryct::trace::Meter(), "foo"};

  std::thread t{[c]() mutable {
    for (int i = 0; i < 100; ++i) {
      c.Increment();
    }
  }};

  for (int i = 0; i < 10; ++i) {
    c.Increment();
  }

  t.join();

  jerryct::trace::PrometheusExporter p{};
  jerryct::trace::Meter().Export(p);

  fmt::memory_buffer content;
  content.reserve(1024);
  p.Expose(content);

  fmt::print(fmt::string_view{content.data(), content.size()});

  return 0;
}
