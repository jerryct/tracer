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
    jerryct::trace::Span _{jerryct::trace::Tracer(), "Async"};
    std::this_thread::sleep_for(std::chrono::milliseconds{42});
  }};
}

void Bar() {
  jerryct::trace::Span _{jerryct::trace::Tracer(), "Bar"};
  std::this_thread::sleep_for(std::chrono::milliseconds{42});
  Async();
}

void Baz() {
  jerryct::trace::Span _{jerryct::trace::Tracer(), "Baz"};
  std::this_thread::sleep_for(std::chrono::milliseconds{100});
}

void Foo() {
  jerryct::trace::Span _{jerryct::trace::Tracer(), "Foo"};
  std::this_thread::sleep_for(std::chrono::milliseconds{23});
  Baz();
}

} // namespace

int main() {
  {
    jerryct::trace::Span _{jerryct::trace::Tracer(), "main"};
    for (int i = 0; i < 10; ++i) {
      std::this_thread::sleep_for(std::chrono::milliseconds{1});
      Foo();
      Bar();
    }

    if (t.joinable()) {
      t.join();
    }
  }

  jerryct::trace::Tracer().Export(jerryct::trace::ChromeTraceEventExporter{"trace_event.json"});

  return 0;
}
