// SPDX-License-Identifier: MIT

#include "jerryct/tracing/chrome_trace_event_exporter.h"
#include "jerryct/tracing/tracing.h"
#include <thread>

namespace {

std::thread t;

void Async() {
  if (t.joinable()) {
    t.join();
  }
  t = std::thread{[]() {
    jerryct::telemetry::Span _{jerryct::telemetry::Tracer(), "Async"};
    std::this_thread::sleep_for(std::chrono::milliseconds{42});
  }};
}

void Bar() {
  jerryct::telemetry::Span _{jerryct::telemetry::Tracer(), "Bar"};
  std::this_thread::sleep_for(std::chrono::milliseconds{42});
  Async();
}

void Baz() {
  jerryct::telemetry::Span _{jerryct::telemetry::Tracer(), "Baz"};
  std::this_thread::sleep_for(std::chrono::milliseconds{100});
}

void Foo() {
  jerryct::telemetry::Span _{jerryct::telemetry::Tracer(), "Foo"};
  std::this_thread::sleep_for(std::chrono::milliseconds{23});
  Baz();
}

} // namespace

int main() {
  {
    jerryct::telemetry::Span _{jerryct::telemetry::Tracer(), "main"};
    for (int i = 0; i < 10; ++i) {
      std::this_thread::sleep_for(std::chrono::milliseconds{1});
      Foo();
      Bar();
    }

    if (t.joinable()) {
      t.join();
    }
  }

  jerryct::telemetry::Tracer().Export(jerryct::telemetry::ChromeTraceEventExporter{"trace_event.json"});

  return 0;
}
