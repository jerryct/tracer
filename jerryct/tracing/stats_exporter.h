// SPDX-License-Identifier: MIT

#ifndef JERRYCT_TRACING_STATS_EXPORTER_H
#define JERRYCT_TRACING_STATS_EXPORTER_H

#include "jerryct/tracing/tracing.h"
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <numeric>
#include <string>
#include <unordered_map>

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

  void operator()(const int tid, const std::uint64_t losts, const std::vector<Event> &events) {
    for (const Event &e : events) {
      switch (e.p) {
      case Phase::begin:
        stacks_[tid].push_back({{e.name.get().data(), e.name.get().size()}, e.ts});
        break;
      case Phase::end:
        if (!stacks_[tid].empty()) {
          const auto d = std::chrono::duration<double, std::milli>{e.ts - stacks_[tid].back().ts}.count();
          data_[stacks_[tid].back().name].push_back(d);
          stacks_[tid].pop_back();
        }
        break;
      }
    }
    losts_[tid] = losts;
  }

  void Print() {
    printf("           min            max           mean   count name\n");
    for (auto &d : data_) {
      const auto mean = std::accumulate(d.second.begin(), d.second.end(), 0.0) / static_cast<double>(d.second.size());
      const auto minmax = std::minmax_element(d.second.begin(), d.second.end());
      printf("%11.6f ms %11.6f ms %11.6f ms %7zu %s\n", *minmax.first, *minmax.second, mean, d.second.size(),
             d.first.c_str());
    }

    std::uint64_t total{};
    for (const auto &l : losts_) {
      total += l.second;
    }
    printf("                                             %7lu total lost event(s)\n", total);
  }

private:
  struct Frame {
    const std::string name;
    const std::chrono::steady_clock::time_point ts;
  };

  std::unordered_map<std::string, std::vector<double>> data_;
  std::unordered_map<int, std::vector<Frame>> stacks_;
  std::unordered_map<int, std::uint64_t> losts_;
};

} // namespace trace
} // namespace jerryct

#endif // JERRYCT_TRACING_STATS_EXPORTER_H
