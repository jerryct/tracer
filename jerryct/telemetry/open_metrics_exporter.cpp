// SPDX-License-Identifier: MIT

#include "jerryct/telemetry/open_metrics_exporter.h"
#include <fmt/core.h>
#include <fmt/format.h>

namespace jerryct {
namespace telemetry {

void OpenMetricsExporter::operator()(const std::unordered_map<string_view, std::uint64_t> &counters) {
  content_.reserve(1024U);
  content_.clear();

  for (const auto &c : counters) {
    content_.append(fmt::string_view{"# TYPE "});
    content_.append(c.first);
    content_.append(fmt::string_view{" counter\n"});
    content_.append(c.first);
    content_.append(fmt::string_view{"_total "});
    content_.append(fmt::format_int{c.second});
    content_.push_back('\n');
  }
}

void OpenMetricsExporter::Expose() {
  if (!server_.IsAlive()) {
    server_ = {};
  }
  server_.Step(content_);
}

} // namespace telemetry
} // namespace jerryct
