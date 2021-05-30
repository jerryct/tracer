// SPDX-License-Identifier: MIT

#include "tracing/chrome_tracing.h"
#include "tracing/tracing.h"
#include <thread>

namespace {

std::thread t;

void Async() {
  if (t.joinable()) {
    t.join();
  }
  t = std::thread{[]() {
    trace::Span s2{trace::Tracer(), "Async"};
    std::this_thread::sleep_for(std::chrono::milliseconds{42});
  }};
}

void Bar() {
  trace::Span s{trace::Tracer(), "Bar"};
  std::this_thread::sleep_for(std::chrono::milliseconds{42});
  Async();
}

void Baz() {
  trace::Span s{trace::Tracer(), "Baz"};
  std::this_thread::sleep_for(std::chrono::milliseconds{100});
}

void Foo() {
  trace::Span s{trace::Tracer(), "Foo"};
  std::this_thread::sleep_for(std::chrono::milliseconds{23});
  Baz();
}

} // namespace

int main() {
  trace::Span s{trace::Tracer(), "main"};
  for (int i = 0; i < 10; ++i) {
    std::this_thread::sleep_for(std::chrono::milliseconds{1});
    Foo();
    Bar();
  }

  if (t.joinable()) {
    t.join();
  }

  s.End();

  trace::Tracer().Export(trace::AsChromeTracingJson{"pretty.json"});

  return 0;
}
