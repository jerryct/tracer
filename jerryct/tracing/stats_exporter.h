// SPDX-License-Identifier: MIT

#ifndef JERRYCT_TELEMETRY_STATS_EXPORTER_H
#define JERRYCT_TELEMETRY_STATS_EXPORTER_H

#include "jerryct/tracing/tracer.h"
#include <chrono>
#include <cstdio>
#include <string>
#include <unordered_map>
#include <vector>

namespace jerryct {
namespace telemetry {

class StatsExporter {
public:
  StatsExporter() = default;
  StatsExporter(const StatsExporter &) = delete;
  StatsExporter(StatsExporter &&other) noexcept = default;
  StatsExporter &operator=(const StatsExporter &) = delete;
  StatsExporter &operator=(StatsExporter &&other) noexcept = default;
  ~StatsExporter() noexcept;

  void operator()(const std::int32_t tid, const std::uint64_t losts, const std::vector<Event> &events);

  void Print();

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
  std::unordered_map<int, std::uint64_t> losts_;
};

} // namespace telemetry
} // namespace jerryct

#endif // JERRYCT_TELEMETRY_STATS_EXPORTER_H
