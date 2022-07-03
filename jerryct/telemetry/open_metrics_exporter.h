// SPDX-License-Identifier: MIT

#ifndef JERRYCT_TELEMETRY_OPEN_METRICS_EXPORTER_H
#define JERRYCT_TELEMETRY_OPEN_METRICS_EXPORTER_H

#include "jerryct/string_view.h"
#include "jerryct/telemetry/http_server.h"
#include <cstdint>
#include <fmt/format.h>
#include <unordered_map>

namespace jerryct {
namespace telemetry {

class OpenMetricsExporter {
public:
  void operator()(const std::unordered_map<string_view, std::uint64_t> &counters);
  void Expose();

private:
  fmt::memory_buffer content_;

  HttpServer server_{};
};

} // namespace telemetry
} // namespace jerryct

#endif // JERRYCT_TELEMETRY_OPEN_METRICS_EXPORTER_H
