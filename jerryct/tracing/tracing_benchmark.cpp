// SPDX-License-Identifier: MIT

#include "jerryct/tracing/tracing.h"
#include <benchmark/benchmark.h>
#include <iostream>
#include <unordered_map>

namespace {

void PerThreadEvents(benchmark::State &state) {
  for (auto _ : state) {
    auto *m = jerryct::trace::Tracer().PerThreadEvents();
    benchmark::DoNotOptimize(m);
    benchmark::ClobberMemory();
  }
}

void Span(benchmark::State &state) {
  auto name = std::string(64, 'c');
  for (auto _ : state) {
    jerryct::trace::Span s{jerryct::trace::Tracer(), name};
    jerryct::trace::Tracer().PerThreadEvents()->events.ConsumeAll([](auto /*unused*/) {});
    benchmark::DoNotOptimize(jerryct::trace::Tracer().PerThreadEvents());
    benchmark::ClobberMemory();
  }
}

void LockFreeQueue(benchmark::State &state) {
  jerryct::trace::LockFreeQueue<int, 4> r{};
  for (auto _ : state) {
    r.Emplace(1);
    r.Emplace(2);
    r.Emplace(3);
    r.ConsumeAll([](int /*unused*/) {});
    benchmark::ClobberMemory();
  }
}

void Metrics(benchmark::State &state) {
  std::cout << "new run\n";
  std::unordered_map<std::string, std::int64_t> c{};

  jerryct::trace::Meter m{jerryct::trace::Tracer(), "foo"};
  for (auto _ : state) {
    m.Increment();
  }

  jerryct::trace::Tracer().Export2([&c](const std::int32_t tid, std::array<jerryct::trace::FixedString, 1024> &names_,
                                        std::array<std::int64_t, 1024> &names2_, int count) {
    for (int i = 0; i < count; ++i)
      c[{names_[i].Get().data(), names_[i].Get().size()}] += names2_[i];
  });

  for (auto &cc : c)
    std::cout << cc.first << ": " << cc.second << "\n";
}

BENCHMARK(PerThreadEvents);
BENCHMARK(Span);
BENCHMARK(LockFreeQueue);
BENCHMARK(Metrics);

} // namespace
