// SPDX-License-Identifier: MIT

#ifndef JERRYCT_TELEMETRY_DELTA_COUNTER_EXPORTER_H
#define JERRYCT_TELEMETRY_DELTA_COUNTER_EXPORTER_H

#include <cstdint>
#include <jerryct/string_view.h>
#include <unordered_map>

namespace jerryct {
namespace telemetry {

class DeltaCounterExporter {
public:
  DeltaCounterExporter();

  void Reset();

  std::uint64_t Get(const jerryct::string_view name);

private:
  std::unordered_map<jerryct::string_view, std::uint64_t> init_;
};

} // namespace telemetry
} // namespace jerryct

#endif // JERRYCT_TELEMETRY_DELTA_COUNTER_EXPORTER_H
