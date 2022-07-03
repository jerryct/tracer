// SPDX-License-Identifier: MIT

#ifndef JERRYCT_TELEMETRY_COUNTER_H
#define JERRYCT_TELEMETRY_COUNTER_H

#include "jerryct/string_view.h"
#include "jerryct/tracing/fixed_string.h"
#include "jerryct/tracing/meter.h"
#include <cstdint>

namespace jerryct {
namespace telemetry {

class Counter final {
public:
  Counter(MeterImpl &t, const jerryct::string_view name);

  void Add();
  void Add(const std::int64_t v);

private:
  MeterImpl *t_;
  const FixedString *id_;
};

} // namespace telemetry
} // namespace jerryct

#endif // JERRYCT_TELEMETRY_COUNTER_H
