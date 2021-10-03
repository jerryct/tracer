// SPDX-License-Identifier: MIT

#ifndef JERRYCT_TRACING_R_EXPORTER_H
#define JERRYCT_TRACING_R_EXPORTER_H

#include "jerryct/tracing/tracing.h"
#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>

namespace jerryct {
namespace trace {

class RExporter {
public:
  explicit RExporter(const std::string &filename);
  RExporter(const RExporter &) = delete;
  RExporter(RExporter &&other) noexcept;
  RExporter &operator=(const RExporter &) = delete;
  RExporter &operator=(RExporter &&other) noexcept;
  ~RExporter() noexcept;

  void operator()(const std::int32_t tid, const std::uint64_t /*unused*/, const std::vector<Event> &events);

private:
  struct Frame {
    const std::string name;
    const std::chrono::steady_clock::time_point ts;
  };

  std::unordered_map<std::string, std::vector<double>> data_;
  std::unordered_map<int, std::vector<Frame>> stacks_;

  int fd_;
};

} // namespace trace
} // namespace jerryct

#endif // JERRYCT_TRACING_R_EXPORTER_H
