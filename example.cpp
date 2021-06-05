// SPDX-License-Identifier: MIT

#include "jerryct/tracing/chrome_tracing.h"
#include "jerryct/tracing/tracing.h"
#include <thread>

namespace {

std::thread t;

void Async() {
  if (t.joinable()) {
    t.join();
  }
  t = std::thread{[]() {
    jerryct::trace::Span s2{jerryct::trace::Tracer(), "Async"};
    std::this_thread::sleep_for(std::chrono::milliseconds{42});
  }};
}

void Bar() {
  jerryct::trace::Span s{jerryct::trace::Tracer(), "Bar"};
  std::this_thread::sleep_for(std::chrono::milliseconds{42});
  Async();
}

void Baz() {
  jerryct::trace::Span s{jerryct::trace::Tracer(), "Baz"};
  std::this_thread::sleep_for(std::chrono::milliseconds{100});
}

void Foo() {
  jerryct::trace::Span s{jerryct::trace::Tracer(), "Foo"};
  std::this_thread::sleep_for(std::chrono::milliseconds{23});
  Baz();
}

} // namespace

int main() {
  {
    jerryct::trace::Span s{jerryct::trace::Tracer(), "main"};
    for (int i = 0; i < 10; ++i) {
      std::this_thread::sleep_for(std::chrono::milliseconds{1});
      Foo();
      Bar();
    }

    if (t.joinable()) {
      t.join();
    }
  }

  jerryct::trace::Tracer().Export(jerryct::trace::AsChromeTracingJson{"pretty.json"});

  return 0;
}
