// SPDX-License-Identifier: MIT

#include "jerryct/telemetry/stats_exporter.h"

namespace jerryct {
namespace telemetry {

StatsExporter::~StatsExporter() noexcept { Print(); }

void StatsExporter::operator()(const std::int32_t tid, const std::uint64_t losts, const std::vector<Event> &events) {
  auto &stack = stacks_[tid];
  for (const Event &e : events) {
    switch (e.phase) {
    case Phase::begin:
      stack.push_back({{e.name.Get().data(), e.name.Get().size()}, e.time_stamp});
      break;
    case Phase::end:
      if (!stack.empty()) {
        auto &data = data_[stack.back().name];
        const auto d = e.time_stamp - stack.back().ts;
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

void StatsExporter::Print() {
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

} // namespace telemetry
} // namespace jerryct
