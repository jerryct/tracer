// SPDX-License-Identifier: MIT

#ifndef JERRYCT_TELEMETRY_METER_H
#define JERRYCT_TELEMETRY_METER_H

#include "jerryct/string_view.h"
#include "jerryct/telemetry/fixed_string.h"
#include "jerryct/telemetry/lock_free_queue.h"
#include "jerryct/telemetry/thread_storage.h"
#include <cstdint>
#include <mutex>
#include <set>
#include <unordered_map>

namespace jerryct {
namespace telemetry {

struct Measurement {
  std::uint64_t value;
  const FixedString *id;
};

class MeterImpl {
public:
  struct Measurements {
    LockFreeQueue<Measurement, 4096> events;
  };

  MeterImpl() { RegisterName("measurement_losts"); }

  template <typename F> void Export(F &&func) {
    std::uint64_t &total_losts{counters_[string_view{"measurement_losts"}]};
    total_losts = 0;

    storage_.Export([&total_losts, &v = counters_](const std::int32_t /*unused*/, Measurements &e) {
      total_losts += e.events.Losts();
      e.events.ConsumeAll([&v](const Measurement &m) { v[m.id->Get()] += m.value; });
    });

    std::forward<F>(func)(static_cast<const std::unordered_map<string_view, std::uint64_t> &>(counters_));
  }

  Measurements *PerThreadEvents() { return storage_.PerThreadEvents(); }

  const FixedString *RegisterName(const string_view name) {
    std::lock_guard<std::mutex> guard{register_names_};
    return &(*names_.emplace(name).first);
  }

private:
  std::mutex register_names_;
  std::set<FixedString> names_;
  std::unordered_map<string_view, std::uint64_t> counters_;
  ThreadStorage<Measurements> storage_;
};

inline MeterImpl &Meter() {
  static MeterImpl *const t = new MeterImpl;
  return *t;
}

} // namespace telemetry
} // namespace jerryct

#endif // JERRYCT_TELEMETRY_METER_H
