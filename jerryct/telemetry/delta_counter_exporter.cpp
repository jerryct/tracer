// SPDX-License-Identifier: MIT

#include "jerryct/telemetry/delta_counter_exporter.h"
#include "jerryct/telemetry/counter.h"
#include "jerryct/telemetry/meter.h"

namespace jerryct {
namespace telemetry {

DeltaCounterExporter::DeltaCounterExporter() { Reset(); }

void DeltaCounterExporter::Reset() {
  Meter().Export([this](const std::unordered_map<jerryct::string_view, std::uint64_t> &counters) { init_ = counters; });
}

std::uint64_t DeltaCounterExporter::Get(const jerryct::string_view name) {
  bool found = false;
  std::uint64_t value = 0;

  Meter().Export([this, name, &found, &value](const std::unordered_map<jerryct::string_view, std::uint64_t> &counters) {
    const auto it = counters.find(name);
    if (it != counters.end()) {
      found = true;
      value = it->second - init_[name];
    }
  });

  return value;
}

} // namespace telemetry
} // namespace jerryct
