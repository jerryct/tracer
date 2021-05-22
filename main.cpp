// SPDX-License-Identifier: MIT

#include "tracing/chrome_tracing.h"
#include "tracing/tracing.h"
#include <thread>

namespace {

std::thread t;

void Async() {
  trace::Token m1 = trace::Tracer().RegisterAsyncEvent("Async", "perf");

  if (t.joinable()) {
    t.join();
  }
  t = std::thread{[m1]() {
    trace::Token m2 = trace::Tracer().RegisterDurationEvent("Async", "perf");
    trace::Span s1(trace::Tracer(), m1);
    trace::Span s2{trace::Tracer(), m2};
    std::this_thread::sleep_for(std::chrono::milliseconds{42});
    s1.End();
  }};
}

void Bar() {
  static trace::Token m = trace::Tracer().RegisterDurationEvent("Bar", "perf");

  trace::Span s{trace::Tracer(), m};
  std::this_thread::sleep_for(std::chrono::milliseconds{42});
  Async();
}

void Baz() {
  static trace::Token m = trace::Tracer().RegisterDurationEvent("Baz", "perf");

  trace::Span s{trace::Tracer(), m};
  std::this_thread::sleep_for(std::chrono::milliseconds{100});
}

void Foo() {
  static trace::Token m = trace::Tracer().RegisterDurationEvent("Foo", "perf");

  trace::Span s{trace::Tracer(), m};
  std::this_thread::sleep_for(std::chrono::milliseconds{23});
  Baz();
}

} // namespace

int main() {
  trace::Token m = trace::Tracer().RegisterDurationEvent("main", "perf");

  trace::Span s{trace::Tracer(), m};
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
