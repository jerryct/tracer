// SPDX-License-Identifier: MIT

#ifndef JERRYCT_TELEMETRY_TRACER_H
#define JERRYCT_TELEMETRY_TRACER_H

#include "jerryct/telemetry/fixed_string.h"
#include "jerryct/telemetry/lock_free_queue.h"
#include "jerryct/telemetry/thread_storage.h"
#include <chrono>
#include <cstdint>
#include <vector>

namespace jerryct {
namespace telemetry {

enum class Phase : std::int32_t { begin, end };

struct Event {
  Phase phase;
  std::chrono::steady_clock::time_point time_stamp;
  FixedString name;
};

class TracerImpl {
public:
  struct Events {
    LockFreeQueue<Event, 4096> events;
  };

  template <typename F> void Export(F &&func) {
    std::vector<Event> v{};
    v.reserve(4096U);

    storage_.Export([&v, &func](std::int32_t tid, Events &e) {
      v.clear();
      e.events.ConsumeAll([&v](const Event &e) { v.push_back(e); });
      func(tid, e.events.Losts(), static_cast<const std::vector<Event> &>(v));
    });
  }

  Events *PerThreadEvents() { return storage_.PerThreadEvents(); }

private:
  ThreadStorage<Events> storage_;
};

inline TracerImpl &Tracer() {
  static TracerImpl *const t = new TracerImpl;
  return *t;
}

} // namespace telemetry
} // namespace jerryct

#endif // JERRYCT_TELEMETRY_TRACER_H
