// SPDX-License-Identifier: MIT

#ifndef JERRYCT_TRACING_STATS_EXPORTER_H
#define JERRYCT_TRACING_STATS_EXPORTER_H

#include "jerryct/tracing/tracing.h"
#include <chrono>
#include <cstdio>
#include <string>
#include <unordered_map>
#include <vector>

namespace jerryct {
namespace trace {

class StatsExporter {
public:
  StatsExporter() = default;
  StatsExporter(const StatsExporter &) = delete;
  StatsExporter(StatsExporter &&other) noexcept = default;
  StatsExporter &operator=(const StatsExporter &) = delete;
  StatsExporter &operator=(StatsExporter &&other) noexcept = default;
  ~StatsExporter() noexcept { Print(); }

  void operator()(const int tid, const std::int64_t losts, const std::vector<Event> &events) {
    auto &stack = stacks_[tid];
    for (const Event &e : events) {
      switch (e.p) {
      case Phase::begin:
        stack.push_back({{e.name.get().data(), e.name.get().size()}, e.ts});
        break;
      case Phase::end:
        if (!stack.empty()) {
          auto &data = data_[stack.back().name];
          const auto d = e.ts - stack.back().ts;
          data.min = d < data.min ? d : data.min;
          data.max = d > data.max ? d : data.max;
          data.sum += d;
          ++data.count;
          stack.pop_back();
        }
        break;
      }
    }
    losts_[tid] = losts;
  }

  void Print() {
    printf("           min            max           mean   count name\n");
    for (auto &d : data_) {
      printf("%11lu ns %11lu ns %11lu ns %7zu %s\n", d.second.min.count(), d.second.max.count(),
             d.second.sum.count() / d.second.count, d.second.count, d.first.c_str());

      d.second.min = std::chrono::nanoseconds::max();
      d.second.max = {};
    }

    std::int64_t total{};
    for (const auto &l : losts_) {
      total += l.second;
    }
    printf("                                             %7ld total lost event(s)\n", total);
  }

private:
  struct Metrics {
    std::chrono::nanoseconds min{std::chrono::nanoseconds::max()};
    std::chrono::nanoseconds max{};
    std::chrono::nanoseconds sum{};
    std::int64_t count{};
  };

  struct Frame {
    const std::string name;
    const std::chrono::steady_clock::time_point ts;
  };

  std::unordered_map<std::string, Metrics> data_;
  std::unordered_map<int, std::vector<Frame>> stacks_;
  std::unordered_map<int, std::int64_t> losts_;
};

} // namespace trace
} // namespace jerryct

#endif // JERRYCT_TRACING_STATS_EXPORTER_H
